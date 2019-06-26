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
#include <QtWidgets/qwidget.h>

class ControlsWidget;
class LayersWidget;
class FramesWidget;
class CellsWidget;

class TimelineWidget final : public QWidget {
  Q_OBJECT
  
  friend class LayerCellsWidget;

public:
  explicit TimelineWidget(QWidget *);
  
  void createInitialCell();
  
public Q_SLOTS:
  void initialize(QSize, Format);
  void save(const QString &) const;
  void load(const QString &);
  void changePalette(Palette *);
  
  void addLayer();
  void removeLayer();
  void moveLayerUp();
  void moveLayerDown();
  void toggleLayerVisible();
  
  void addFrame();
  void addNullFrame();
  void removeFrame();
  
  void requestCell();
  void toggleAnimation();
  
  void layerAbove();
  void layerBelow();
  void nextFrame();
  void prevFrame();

Q_SIGNALS:
  // emitted when the current layer/frame has changed
  // or the current cell has changed
  void posChanged(Cell *, LayerIdx, FrameIdx);
  void visibleChanged(const LayerVisible &);
  void frameChanged(const Frame &, QSize, Format);

private Q_SLOTS:
  void changeFrame(const Frame &);

private:
  ControlsWidget *controls = nullptr;
  LayersWidget *layers = nullptr;
  FramesWidget *frames = nullptr;
  CellsWidget *cells = nullptr;
  Palette *palette = nullptr;
  QSize size;
  Format format;
};

#endif
