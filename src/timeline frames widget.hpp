//
//  timeline frames widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 24/6/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_timeline_frames_widget_hpp
#define animera_timeline_frames_widget_hpp

#include "cell.hpp"
#include <QtWidgets/qscrollarea.h>

class FramesWidget final : public QWidget {
  Q_OBJECT

public:
  explicit FramesWidget(QWidget *);

public Q_SLOTS:
  void setFrameCount(FrameIdx);
  void setMargin(int);

private:
  FrameIdx frames = {};
  int margin = 0;
  
  int roundUpFrames() const;
  void setWidth();
  
  void paintEvent(QPaintEvent *);
};

class FrameScrollWidget final : public QScrollArea {
  Q_OBJECT
  
public:
  explicit FrameScrollWidget(QWidget *);

  FramesWidget *getChild();

Q_SIGNALS:
  void shouldSetRightMargin(int);

private:
  void paintEvent(QPaintEvent *) override;
};

#endif
