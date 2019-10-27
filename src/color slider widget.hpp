//
//  color slider widget.hpp
//  Animera
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef color_slider_widget_hpp
#define color_slider_widget_hpp

#include "color convert.hpp"
#include <QtWidgets/qwidget.h>

template <typename Derived>
class ColorSliderWidget : public QWidget {
public:
  explicit ColorSliderWidget(QWidget *);

protected:
  QImage graph;

private:
  QPixmap bar;
  bool mouseDown = false;
  
  void initBar();
  void renderGraph(QPainter &);
  void renderBar(QPainter &);
  
  void paintEvent(QPaintEvent *) override;
  
  void setColor(QMouseEvent *);
  
  void mousePressEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void keyPressEvent(QKeyEvent *) override;
};

class HueSliderWidget final : public ColorSliderWidget<HueSliderWidget> {
  Q_OBJECT
  
  friend class ColorSliderWidget<HueSliderWidget>;

public:
  explicit HueSliderWidget(QWidget *);

Q_SIGNALS:
  void hueChanged(int);

public Q_SLOTS:
  void setHue(int);
  void setSV(int, int);
  void setHSV(HSV);
  
private:
  HSV color;
  
  void plotGraph();
  void renderBackground(QPainter &);
  
  void updateHue(int);
  int getPixel();
  void setColor(int);
  void incColor();
  void decColor();
};

class AlphaSliderWidget final : public ColorSliderWidget<AlphaSliderWidget> {
  Q_OBJECT
  
  friend class ColorSliderWidget<AlphaSliderWidget>;
  
public:
  explicit AlphaSliderWidget(QWidget *);
  
Q_SIGNALS:
  void alphaChanged(int);

public Q_SLOTS:
  void setAlpha(int);
  void setHue(int);
  void setSV(int, int);
  void setHSV(HSV);
  
private:
  HSV color;
  int alpha;
  
  void plotGraph();
  void renderBackground(QPainter &);
  
  void updateAlpha(int);
  int getPixel();
  void setColor(int);
  void incColor();
  void decColor();
};

class GraySliderWidget final : public ColorSliderWidget<GraySliderWidget> {
  Q_OBJECT
  
  friend class ColorSliderWidget<GraySliderWidget>;

public:
  explicit GraySliderWidget(QWidget *);

Q_SIGNALS:
  void grayChanged(int);

public Q_SLOTS:
  void setGray(int);

private:
  int gray;
  
  void plotGraph();
  void renderBackground(QPainter &);
  
  void updateGray(int);
  int getPixel();
  void setColor(int);
  void incColor();
  void decColor();
};

#endif
