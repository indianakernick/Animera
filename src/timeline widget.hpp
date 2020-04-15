//
//  timeline widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 10/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_timeline_widget_hpp
#define animera_timeline_widget_hpp

#include <span>
#include "cell.hpp"
#include <QtWidgets/qwidget.h>

class ControlsWidget;
class LayersWidget;
class FramesWidget;
class CellsWidget;
class StatusObject;

class TimelineWidget final : public QWidget {
  Q_OBJECT
  
public:
  explicit TimelineWidget(QWidget *);
  
public Q_SLOTS:
  void setCurrPos(CellPos);
  void setSelection(CellRect);
  void setVisibility(LayerIdx, bool);
  void isolateLayer(LayerIdx);
  void setName(LayerIdx, std::string_view);
  void setLayer(LayerIdx, std::span<const CellSpan>);
  void setFrameCount(FrameIdx);
  void setLayerCount(LayerIdx);
  void toggleAnimation();
  void setDelay(int);

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
  void delayChanged(int);
  void animationToggled(bool);
  
private:
  ControlsWidget *controls = nullptr;
  LayersWidget *layers = nullptr;
  FramesWidget *frames = nullptr;
  CellsWidget *cells = nullptr;
};

#endif
