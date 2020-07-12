//
//  timeline cels widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 24/6/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_timeline_cels_widget_hpp
#define animera_timeline_cels_widget_hpp

#include "cel.hpp"
#include "cel span.hpp"
#include "group array.hpp"
#include <QtCore/qtimer.h>
#include "cel span painter.hpp"
#include "scroll bar widget.hpp"

// TODO: Share CelSpanPainter between CelsWidget and GroupsWidget

class CelsWidget final : public QWidget {
  Q_OBJECT
  
public:
  explicit CelsWidget(QWidget *);

Q_SIGNALS:
  void resized();
  void shouldEnsureVisible(QPoint);
  void shouldBeginSelection();
  void shouldContinueSelection();
  void shouldEndSelection();
  void shouldClearSelection();
  void shouldSetPos(CelPos);
  
public Q_SLOTS:
  void setSelection(CelRect);
  void setPos(CelPos);
  void setLayer(LayerIdx, tcb::span<const CelSpan>);
  void setFrameCount(FrameIdx);
  void setLayerCount(LayerIdx);

private:
  CelSpanPainter celPainter;
  QImage posImg;
  QImage selectionImg;
  QImage bordersImg;
  QImage layersImg;
  
  QRect selectionRect;
  
  void updateSelectionImg();

  void paintEvent(QPaintEvent *) override;
  void focusOutEvent(QFocusEvent *) override;
  
  CelPos getPos(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
};

class CelScrollWidget final : public ScrollAreaWidget {
  Q_OBJECT
  
public:
  explicit CelScrollWidget(QWidget *);

  CelsWidget *getChild();

Q_SIGNALS:
  void rightMarginChanged(int);
  void bottomMarginChanged(int);

public Q_SLOTS:
  void changeMargins();
  void ensureVisible(QPoint);

private:
  QWidget *rect = nullptr;

  void resizeEvent(QResizeEvent *) override;
};

class GroupsWidget final : public QWidget {
  Q_OBJECT

public:
  explicit GroupsWidget(QWidget *);

Q_SIGNALS:
  void shouldSetGroup(FrameIdx);
  void shouldMoveGroup(GroupIdx, FrameIdx);

public Q_SLOTS:
  void setGroupArray(tcb::span<const Group>);
  void setGroup(GroupInfo);
  void setFrameCount(FrameIdx);
  void setMargin(int);

private:
  CelSpanPainter celPainter;
  QImage groupImg;
  QImage selectionImg;
  tcb::span<const Group> groupArray;
  FrameIdx frames = {};
  int margin = 0;
  std::optional<GroupIdx> dragGroup;

  void setWidth();
  int clampPos(QMouseEvent *) const;
  FrameIdx framePos(int) const;
  std::optional<FrameIdx> boundaryPos(int) const;

  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void paintEvent(QPaintEvent *) override;
};

class GroupScrollWidget final : public QScrollArea {
  Q_OBJECT

public:
  explicit GroupScrollWidget(QWidget *);

  GroupsWidget *getChild();

Q_SIGNALS:
  void shouldSetRightMargin(int);

private:
  void paintEvent(QPaintEvent *) override;
};

#endif
