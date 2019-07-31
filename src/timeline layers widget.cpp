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
#include "timeline painting.hpp"
#include "text input widget.hpp"
#include <QtWidgets/qboxlayout.h>

LayerNameWidget::LayerNameWidget(QWidget *parent, const LayerIdx layer)
  : QWidget{parent}, idx{layer} {
  setFixedSize(layer_width, cell_height);
  createWidgets();
  setupLayout();
  CONNECT(visible, toggled, this, changeVisibility);
  CONNECT(name, textEdited, this, changeName);
}

void LayerNameWidget::setVisibility(const bool visibility) {
  visible->setChecked(visibility);
}

void LayerNameWidget::setName(const std::string_view text) {
  name->setText(QString{QLatin1String{text.data(), static_cast<int>(text.size())}});
}

void LayerNameWidget::changeVisibility(const bool visibility) {
  Q_EMIT visibilityChanged(idx, visibility);
}

void LayerNameWidget::changeName(const QString &text) {
  Q_EMIT nameChanged(idx, text.toStdString());
}

void LayerNameWidget::createWidgets() {
  QPixmap onPix{cell_width, cell_width};
  paintTimelineButtonBack(onPix);
  QPixmap offPix = onPix;
  paintTimelineButtonIcon(onPix, ":/Timeline/shown.pbm");
  paintTimelineButtonIcon(offPix, ":/Timeline/hidden.pbm");
  visible = new IconRadioButtonWidget{this, onPix, offPix};
  name = new TextInputWidget{this, layer_text_rect};
}

void LayerNameWidget::setupLayout() {
  auto *layout = new QHBoxLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setAlignment(Qt::AlignLeft);
  layout->addWidget(visible);
  layout->addWidget(name);
}

LayersWidget::LayersWidget(QWidget *parent)
  : QWidget{parent}, layout{new QVBoxLayout{this}} {
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setAlignment(Qt::AlignTop);
  layout->setSizeConstraint(QLayout::SetFixedSize);
}

void LayersWidget::setMargin(const int margin) {
  layout->setContentsMargins(0, 0, 0, margin);
}

void LayersWidget::setVisibility(const LayerIdx idx, const bool visible) {
  layers[+idx]->setVisibility(visible);
}

void LayersWidget::setName(const LayerIdx idx, const std::string_view name) {
  layers[+idx]->setName(name);
}

void LayersWidget::setLayerCount(const LayerIdx count) {
  while (layerCount() > count) {
    delete layers.back();
    layers.pop_back();
  }
  while (layerCount() < count) {
    auto *layerName = new LayerNameWidget{this, layerCount()};
    CONNECT(layerName, visibilityChanged, this, visibilityChanged);
    CONNECT(layerName, nameChanged,       this, nameChanged);
    layout->addWidget(layerName);
    layers.push_back(layerName);
  }
}

LayerIdx LayersWidget::layerCount() const {
  return static_cast<LayerIdx>(layers.size());
}

LayerScrollWidget::LayerScrollWidget(QWidget *parent)
  : QScrollArea{parent} {
  setFrameShape(NoFrame);
  setFixedWidth(layer_width);
  setStyleSheet("background-color:" + glob_main.name());
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setMinimumHeight(cell_height + glob_scroll_width);
}

LayersWidget *LayerScrollWidget::getChild() {
  auto *layers = new LayersWidget{this};
  // We cannot simply call setViewportMargins
  CONNECT(this, shouldSetBottomMargin, layers, setMargin);
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
