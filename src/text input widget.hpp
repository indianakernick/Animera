//
//  text input widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef text_input_widget_hpp
#define text_input_widget_hpp

#include "config.hpp"
#include <QtCore/qtimer.h>
#include <QtWidgets/qlineedit.h>

class TextInputWidget : public QLineEdit {
  Q_OBJECT
  
public:
  TextInputWidget(QWidget *, WidgetRect);

private Q_SLOTS:
  void blinkSlot();
  void showCursor();
  void hideCursor();

private:
  QTimer cursorBlinkTimer;
  WidgetRect rect;
  bool cursorBlinkStatus = true;
  
  void focusInEvent(QFocusEvent *) override;

  void renderBackground(QPainter &);
  void renderText(QPainter &);
  void renderCursor(QPainter &);
  void renderSelection(QPainter &);

  void paintEvent(QPaintEvent *) override;
};

#endif
