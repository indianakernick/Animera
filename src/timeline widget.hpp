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
  void setPos(CellPos);
  void setSelection(CellRect);
  void setVisibility(LayerIdx, bool);
  void setName(LayerIdx, std::string_view);
  void setLayer(LayerIdx, std::span<const CellSpan>);
  void setFrameCount(FrameIdx);
  void setLayerCount(LayerIdx);
  void toggleAnimation();
  void setDelay(int);

Q_SIGNALS:
  void shouldSetVisibility(LayerIdx, bool);
  void shouldIsolateVisibility(LayerIdx);
  void shouldSetName(LayerIdx, std::string_view);
  
  void shouldNextFrame();
  void shouldInsertLayer();
  void shouldRemoveLayer();
  void shouldMoveLayerUp();
  void shouldMoveLayerDown();
  void shouldExtendCell();
  void shouldSplitCell();
  void shouldSetDelay(int);
  void shouldToggleAnimation(bool);
  
  void shouldBeginSelection();
  void shouldContinueSelection();
  void shouldEndSelection();
  void shouldClearSelection();
  void shouldSetPos(CellPos);
  
private:
  ControlsWidget *controls = nullptr;
  LayersWidget *layers = nullptr;
  FramesWidget *frames = nullptr;
  CellsWidget *cells = nullptr;
};

#endif
