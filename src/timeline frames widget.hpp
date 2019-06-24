//
//  timeline frames widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef timeline_frames_widget_hpp
#define timeline_frames_widget_hpp

#include <QtWidgets/qscrollarea.h>

class FramesWidget final : public QWidget {
  Q_OBJECT

public:
  explicit FramesWidget(QWidget *);
 
  void appendFrame();

Q_SIGNALS:
  void widthChanged(int);
 
private:
  int frames = 0;
  
  int roundUpFrames() const;
  
  void paintEvent(QPaintEvent *);
};

class FrameScrollWidget final : public QScrollArea {
  Q_OBJECT
  
public:
  explicit FrameScrollWidget(QWidget *);

public Q_SLOTS:
  void changeRightMargin(int);

private:
  void paintEvent(QPaintEvent *) override;
};

#endif
