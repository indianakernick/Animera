//
//  text input widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef text_input_widget_hpp
#define text_input_widget_hpp

#include <QtCore/qtimer.h>
#include "rect widget size.hpp"
#include <QtWidgets/qlineedit.h>

class TextInputWidget : public QLineEdit {
  Q_OBJECT
  
public:
  TextInputWidget(QWidget *, RectWidgetSize, int);

private Q_SLOTS:
  void blinkSlot();
  void showCursor();

private:
  QTimer cursorBlinkTimer;
  RectWidgetSize boxSize;
  int offsetX;
  bool cursorBlinkStatus = true;
  
  void focusInEvent(QFocusEvent *) override;

  void renderBackground(QPainter &);
  void renderText(QPainter &);
  void renderCursor(QPainter &);
  void renderSelection(QPainter &painter);

  void paintEvent(QPaintEvent *) override;
};

#endif
