//
//  timeline widget.hpp
//  Animera
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef timeline_widget_hpp
#define timeline_widget_hpp

#include <span>
#include "cell.hpp"
#include <QtWidgets/qwidget.h>

class ControlsWidget;
class LayersWidget;
class FramesWidget;
class CellsWidget;

class TimelineWidget final : public QWidget {
  Q_OBJECT
  
public:
  explicit TimelineWidget(QWidget *);
  
public Q_SLOTS:
  void setCurrPos(CellPos);
  void setSelection(CellRect);
  void setVisibility(LayerIdx, bool);
  void setName(LayerIdx, std::string_view);
  void setLayer(LayerIdx, std::span<const CellSpan>);
  void setFrameCount(FrameIdx);
  void setLayerCount(LayerIdx);
  void toggleAnimation();

Q_SIGNALS:
  void visibilityChanged(LayerIdx, bool);
  void nameChanged(LayerIdx, std::string_view);
  
  void nextFrame();
  void insertLayer();
  void removeLayer();
  void moveLayerUp();
  void moveLayerDown();
  void extendCell();
  void splitCell();
  
  void beginSelection();
  void continueSelection();
  void endSelection();
  void clearSelection();
  void currPosChanged(CellPos);
  
private:
  ControlsWidget *controls = nullptr;
  LayersWidget *layers = nullptr;
  FramesWidget *frames = nullptr;
  CellsWidget *cells = nullptr;
};

#endif
