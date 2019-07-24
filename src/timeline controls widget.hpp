//
//  timeline controls widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef timeline_controls_widget_hpp
#define timeline_controls_widget_hpp

#include <QtCore/qtimer.h>
#include "color input widget.hpp"
#include "icon push button widget.hpp"
#include "icon radio button widget.hpp"

class ControlsWidget final : public QWidget {
  Q_OBJECT

public:
  explicit ControlsWidget(QWidget *);

Q_SIGNALS:
  void nextFrame();
  void insertLayer();
  void removeLayer();
  void moveLayerUp();
  void moveLayerDown();
  void extendCell();
  void splitCell();

public Q_SLOTS:
  void toggleAnimation();

private:
  QTimer animTimer;
  IconPushButtonWidget *insertLayerButton;
  IconPushButtonWidget *removeLayerButton;
  IconPushButtonWidget *moveLayerUpButton;
  IconPushButtonWidget *moveLayerDownButton;
  IconPushButtonWidget *extendButton;
  IconPushButtonWidget *splitButton;
  IconRadioButtonWidget *playButton;
  NumberInputWidget *delayBox;
  
  void toggleTimer();
  void setInterval(int);
  
  void paintBack(QPixmap &);
  void paintIcon(QPixmap &, const QString &);
  IconPushButtonWidget *makePushButton(QPixmap, const QString &);
  IconRadioButtonWidget *makeRadioButton(QPixmap, const QString &, const QString &);
  void createWidgets();
  void setupLayout();
  void connectSignals();
  
  void paintEvent(QPaintEvent *) override;
};

#endif