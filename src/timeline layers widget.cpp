//
//  timeline layers widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline layers widget.hpp"

#include "serial.hpp"
#include "config.hpp"
#include "connect.hpp"
#include <QtGui/qpainter.h>
#include "widget painting.hpp"
#include "text input widget.hpp"
#include <QtWidgets/qboxlayout.h>

VisibleWidget::VisibleWidget(QWidget *parent)
  : QAbstractButton{parent} {
  setCheckable(true);
  setChecked(true);
  setFixedSize(cell_icon_step, cell_icon_step);
  loadIcons();
}

void VisibleWidget::loadIcons() {
  // @TODO cache
  shownPix = bakeColoredBitmap(":/Timeline/shown.pbm", cell_icon_color);
  hiddenPix = bakeColoredBitmap(":/Timeline/hidden.pbm", cell_icon_color);
}

void VisibleWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.fillRect(0, 0, width() - glob_border_width, height() - glob_border_width, glob_main);
  QPixmap pixmap = isChecked() ? shownPix : hiddenPix;
  painter.drawPixmap(cell_icon_pad, cell_icon_pad, pixmap);
  painter.fillRect(
    width() - glob_border_width, 0,
    glob_border_width, height(),
    glob_border_color
  );
}

LayerNameWidget::LayerNameWidget(QWidget *parent, const LayerIdx layer)
  : QWidget{parent} {
  setFixedSize(101_px, cell_height);
  setupLayout();
  name->setText("Layer " + QString::number(layer));
  CONNECT(visible, toggled, this, visibleToggled);
}

bool LayerNameWidget::getVisible() const {
  return visible->isChecked();
}

QString LayerNameWidget::getName() const {
  return name->text();
}

void LayerNameWidget::clearInfo() {
  visible->setChecked(true);
  name->setText("Layer 0");
}

void LayerNameWidget::swapWith(LayerNameWidget &other) {
  bool tempChecked = visible->isChecked();
  QString tempName = name->text();
  visible->setChecked(other.visible->isChecked());
  name->setText(other.name->text());
  other.visible->setChecked(tempChecked);
  other.name->setText(tempName);
}

void LayerNameWidget::toggleVisible() {
  visible->toggle();
}

void LayerNameWidget::serialize(QIODevice *dev) const {
  assert(dev);
  serializeBytes(dev, visible->isChecked());
  const QString text = name->text();
  assert(text.size() < 0xFFFF);
  serializeBytes(dev, static_cast<uint16_t>(text.size()));
  dev->write(reinterpret_cast<const char *>(text.data()), text.size() * sizeof(QChar));
}

void LayerNameWidget::deserialize(QIODevice *dev) {
  assert(dev);
  bool isVisible;
  deserializeBytes(dev, isVisible);
  visible->setChecked(isVisible);
  uint16_t nameSize;
  deserializeBytes(dev, nameSize);
  QString text;
  text.resize(nameSize);
  dev->read(reinterpret_cast<char *>(text.data()), nameSize * sizeof(QChar));
  name->setText(text);
  repaint();
}

void LayerNameWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.fillRect(
    0, cell_height - glob_border_width,
    width(), glob_border_width,
    glob_border_color
  );
  painter.fillRect(
    width() - glob_border_width, 0,
    glob_border_width, height(),
    glob_border_color
  );
}

void LayerNameWidget::setupLayout() {
  QHBoxLayout *layout = new QHBoxLayout{this};
  setLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setAlignment(Qt::AlignLeft);
  visible = new VisibleWidget{this};
  layout->addWidget(visible);
  constexpr int text_width = 101_px - cell_icon_step - glob_border_width;
  constexpr int text_height = cell_height - glob_border_width;
  name = new TextInputWidget{this, {{1_px, 2_px}, {text_width, text_height}, 0, 0}};
  layout->addWidget(name, 0, Qt::AlignTop);
}

LayersWidget::LayersWidget(QWidget *parent)
  : QWidget{parent}, layout{new QVBoxLayout{this}} {
  setLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setAlignment(Qt::AlignTop);
  layout->setSizeConstraint(QLayout::SetFixedSize);
}

void LayersWidget::appendLayer(const LayerIdx layer) {
  auto *layerName = new LayerNameWidget{this, layer};
  CONNECT(layerName, visibleToggled, this, changeVisible);
  CONNECT(layerName, visibleToggled, this, composite);
  layers.push_back(layerName);
  layout->addWidget(layerName);
  changeVisible();
}

void LayersWidget::serialize(QIODevice *dev) const {
  assert(dev);
  serializeBytes(dev, static_cast<uint16_t>(layers.size()));
  for (const LayerNameWidget *layer : layers) {
    layer->serialize(dev);
  }
}

void LayersWidget::deserialize(QIODevice *dev) {
  assert(dev);
  uint16_t layersSize;
  deserializeBytes(dev, layersSize);
  for (LayerNameWidget *layer : layers) {
    delete layer;
  }
  layers.clear();
  layers.reserve(layersSize);
  for (LayerIdx l = 0; l != layersSize; ++l) {
    auto *layer = new LayerNameWidget{this, l};
    layer->deserialize(dev);
    CONNECT(layer, visibleToggled, this, changeVisible);
    CONNECT(layer, visibleToggled, this, composite);
    layers.push_back(layer);
    layout->addWidget(layer);
  }
  changeVisible();
}

void LayersWidget::changeVisible() {
  LayerVisible visible;
  visible.reserve(layers.size());
  for (LayerNameWidget *layer : layers) {
    visible.push_back(layer->getVisible());
  }
  Q_EMIT visibleChanged(visible);
}

void LayersWidget::setMargin(const int margin) {
  layout->setContentsMargins(0, 0, 0, margin);
}

void LayersWidget::insertLayer(const LayerIdx idx) {
  auto *layer = new LayerNameWidget{this, static_cast<LayerIdx>(layers.size())};
  CONNECT(layer, visibleToggled, this, changeVisible);
  CONNECT(layer, visibleToggled, this, composite);
  layers.insert(layers.begin() + idx, layer);
  layout->insertWidget(idx, layer);
  changeVisible();
}

void LayersWidget::removeLayer(const LayerIdx idx) {
  if (layers.size() == 1) {
    layers[idx]->clearInfo();
  } else {
    for (size_t l = idx; l < layers.size() - 1; ++l) {
      layers[l]->swapWith(*layers[l + 1]);
    }
    layout->removeWidget(layers.back());
    delete layers.back();
    layers.pop_back();
  }
  changeVisible();
}

void LayersWidget::moveLayerUp(const LayerIdx idx) {
  if (idx == 0) return;
  LayerNameWidget *layer = layers[idx];
  std::swap(layers[idx - 1], layers[idx]);
  layout->removeWidget(layer);
  layout->insertWidget(idx - 1, layer);
  changeVisible();
}

void LayersWidget::moveLayerDown(const LayerIdx idx) {
  if (idx == static_cast<LayerIdx>(layers.size()) - 1) return;
  LayerNameWidget *layer = layers[idx];
  std::swap(layers[idx], layers[idx + 1]);
  layout->removeWidget(layer);
  layout->insertWidget(idx + 1, layer);
  changeVisible();
}

void LayersWidget::toggleVisible(const LayerIdx idx) {
  layers[idx]->toggleVisible();
}

LayerScrollWidget::LayerScrollWidget(QWidget *parent)
  : QScrollArea{parent} {
  setFrameShape(NoFrame);
  setFixedWidth(101_px);
  setStyleSheet("background-color:" + glob_main.name());
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setMinimumHeight(cell_height + glob_scroll_width);
}

LayersWidget *LayerScrollWidget::setChild(LayersWidget *layers) {
  // We cannot simply call setViewportMargins
  CONNECT(this, changeBottomMargin, layers, setMargin);
  setWidget(layers);
  return layers;
}

void LayerScrollWidget::paintEvent(QPaintEvent *) {
  QPainter painter{viewport()};
  painter.fillRect(
    width() - glob_border_width, 0,
    glob_border_width, height(),
    glob_border_color
  );
}

#include "timeline layers widget.moc"
