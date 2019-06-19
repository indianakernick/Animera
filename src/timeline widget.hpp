//
//  timeline widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef timeline_widget_hpp
#define timeline_widget_hpp

#include "animation.hpp"
#include <QtWidgets/qscrollarea.h>

class LayerWidget;

class TimelineWidget final : public QScrollArea {
  Q_OBJECT
  
  friend LayerWidget;

public:
  explicit TimelineWidget(QWidget *);
  
  void createInitialCell();
  
public Q_SLOTS:
  void initialize(QSize, Format);
  void save(const QString &) const;
  void load(const QString &);
  void paletteChanged(Palette *);

Q_SIGNALS:
  // emitted when the current layer/frame has changed
  // or the current cell has changed
  void posChange(Cell *, LayerIdx, FrameIdx);
  // emitted when any layers are shown or hidden
  void layerVisibility(const LayerVisible &);
  void frameChanged(const Frame &);

private:
  std::vector<LayerWidget *> layers;
  Palette *palette = nullptr;
  QSize size;
  Format format;
};

#endif
