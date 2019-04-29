//
//  color graph widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef color_graph_widget_hpp
#define color_graph_widget_hpp

#include "color convert.hpp"
#include <QtWidgets/qwidget.h>

class SVGraphWidget final : public QWidget {
  Q_OBJECT

public:
  explicit SVGraphWidget(QWidget *);

Q_SIGNALS:
  void svChanged(int, int);

public Q_SLOTS:
  void changeHue(int);
  void changeSV(int, int);
  void changeHSV(HSV);

private:
  QImage graph;
  QPixmap circle;
  HSV color;
  bool mouseDown = false;
  
  void plotGraph(int);
  void initCircle();
  
  void renderGraph(QPainter &);
  void renderCircle(QPainter &);
  
  void paintEvent(QPaintEvent *) override;
  
  void setColor(QPointF);
  
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
};

#endif
