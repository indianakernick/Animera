//
//  timeline layers widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline layers widget.hpp"

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
  setFixedSize(100_px, cell_height);
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
  constexpr int text_width = 100_px - cell_icon_step - glob_border_width;
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
  CONNECT(layerName, visibleToggled, this, toggleVisible);
  layers.push_back(layerName);
  layout->addWidget(layerName);
  toggleVisible();
}

void LayersWidget::toggleVisible() {
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
  CONNECT(layer, visibleToggled, this, toggleVisible);
  layers.insert(layers.begin() + idx, layer);
  layout->insertWidget(idx, layer);
  toggleVisible();
}

void LayersWidget::removeLayer(const LayerIdx idx) {
  LayerNameWidget *layer = layers[idx];
  if (layers.size() == 1) {
    layer->clearInfo();
  } else {
    layout->removeWidget(layer);
    layers.erase(layers.begin() + idx);
    delete layer;
  }
  toggleVisible();
}

void LayersWidget::moveLayerUp(const LayerIdx idx) {
  if (idx == 0) return;
  LayerNameWidget *layer = layers[idx];
  std::swap(layers[idx - 1], layers[idx]);
  layout->removeWidget(layer);
  layout->insertWidget(idx - 1, layer);
  toggleVisible();
}

void LayersWidget::moveLayerDown(const LayerIdx idx) {
  if (idx == static_cast<LayerIdx>(layers.size()) - 1) return;
  LayerNameWidget *layer = layers[idx];
  std::swap(layers[idx], layers[idx + 1]);
  layout->removeWidget(layer);
  layout->insertWidget(idx + 1, layer);
  toggleVisible();
}

LayerScrollWidget::LayerScrollWidget(QWidget *parent)
  : QScrollArea{parent} {
  setFrameShape(NoFrame);
  setFixedWidth(100_px);
  setStyleSheet("background-color:" + glob_main.name());
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
