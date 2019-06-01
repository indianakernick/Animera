//
//  editor widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef editor_widget_hpp
#define editor_widget_hpp

#include "tool.hpp"
#include "animation.hpp"
#include <QtWidgets/qscrollarea.h>

class EditorImage;

class EditorWidget final : public QScrollArea {
  Q_OBJECT

public:
  EditorWidget(QWidget *, Animation &);

Q_SIGNALS:
  void mouseLeave(QImage *);
  void mouseDown(QPoint, ButtonType, QImage *);
  void mouseMove(QPoint, QImage *);
  void mouseUp(QPoint, ButtonType, QImage *);
  void keyPress(Qt::Key, QImage *);

public Q_SLOTS:
  // invoked when the current frame has changed
  // (from a paint tool or a timeline operation)
  void composite();
  void compositeOverlay();
  // invoked when the image needs to be composited
  void compositePos(Cell *, LayerIdx, FrameIdx);
  // invoked when the image needs to be composited
  void compositeVis(const LayerVisible &);

private:
  Animation &anim;
  EditorImage *view;
  LayerVisible visibility;
  LayerIdx layer = 0;
  FrameIdx frame = 0;
  
  void enterEvent(QEvent *) override;
  void leaveEvent(QEvent *) override;
};

#endif
