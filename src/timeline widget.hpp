//
//  timeline widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 10/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_timeline_widget_hpp
#define animera_timeline_widget_hpp

#include "cel.hpp"
#include "cel span.hpp"
#include "group array.hpp"
#include <QtWidgets/qwidget.h>

class ControlsWidget;
class GroupNameWidget;
class LayersWidget;
class FramesWidget;
class GroupsWidget;
class CelsWidget;
class StatusObject;

class TimelineWidget final : public QWidget {
  Q_OBJECT
  
public:
  explicit TimelineWidget(QWidget *);
  
public Q_SLOTS:
  void setPos(CelPos);
  void setSelection(CelRect);
  void setGroup(GroupInfo);
  void setGroupName(std::string_view);
  void setGroupArray(tcb::span<const Group>);
  void setVisibility(LayerIdx, bool);
  void setLayerName(LayerIdx, std::string_view);
  void setLayer(LayerIdx, tcb::span<const CelSpan>);
  void setFrameCount(FrameIdx);
  void setLayerCount(LayerIdx);
  void toggleAnimation();
  void setDelay(int);

Q_SIGNALS:
  void shouldSetVisibility(LayerIdx, bool);
  void shouldIsolateVisibility(LayerIdx);
  void shouldSetLayerName(LayerIdx, std::string_view);
  
  void shouldNextFrame();
  void shouldInsertLayer();
  void shouldRemoveLayer();
  void shouldMoveLayerUp();
  void shouldMoveLayerDown();
  void shouldExtendCel();
  void shouldSplitCel();
  void shouldSetDelay(int);
  void shouldToggleAnimation(bool);
  
  void shouldBeginSelection();
  void shouldContinueSelection();
  void shouldEndSelection();
  void shouldClearSelection();
  void shouldSetPos(CelPos);
  
  void shouldSetGroup(FrameIdx);
  void shouldSetGroupName(std::string_view);
  void shouldMoveGroup(GroupIdx, FrameIdx);
  
private:
  ControlsWidget *controls = nullptr;
  GroupNameWidget *groupName = nullptr;
  LayersWidget *layers = nullptr;
  FramesWidget *frames = nullptr;
  GroupsWidget *groups = nullptr;
  CelsWidget *cels = nullptr;
};

#endif
