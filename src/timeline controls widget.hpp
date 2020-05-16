//
//  timeline controls widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 7/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_timeline_controls_widget_hpp
#define animera_timeline_controls_widget_hpp

#include <QtCore/qtimer.h>
#include "color input widget.hpp"
#include "icon push button widget.hpp"
#include "icon radio button widget.hpp"

class ControlsWidget final : public QWidget {
  Q_OBJECT

public:
  explicit ControlsWidget(QWidget *);

Q_SIGNALS:
  void shouldNextFrame();
  void shouldInsertLayer();
  void shouldRemoveLayer();
  void shouldMoveLayerUp();
  void shouldMoveLayerDown();
  void shouldExtendCel();
  void shouldSplitCel();
  void shouldSetDelay(int);
  void shouldToggleAnimation(bool);

public Q_SLOTS:
  void toggleAnimation();
  void setDelay(int);

private Q_SLOTS:
  void toggleTimer();
  void changeDelay(int);

private:
  QTimer animTimer;
  IconPushButtonWidget *insertLayerButton = nullptr;
  IconPushButtonWidget *removeLayerButton = nullptr;
  IconPushButtonWidget *moveLayerUpButton = nullptr;
  IconPushButtonWidget *moveLayerDownButton = nullptr;
  IconPushButtonWidget *extendButton = nullptr;
  IconPushButtonWidget *splitButton = nullptr;
  IconRadioButtonWidget *playButton = nullptr;
  NumberInputWidget *delayBox = nullptr;
  
  IconPushButtonWidget *makePushButton(QPixmap, const QString &);
  IconRadioButtonWidget *makeRadioButton(QPixmap, const QString &, const QString &);
  void createWidgets();
  void setupLayout();
  void connectSignals();
};

#endif
