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
  void blink();
  void showCursor();
  void hideCursor();

private:
  WidgetRect rect;
  int offset = 0;
  QTimer cursorBlinkTimer;
  bool cursorBlinkStatus = true;
  
  void focusInEvent(QFocusEvent *) override;
  void focusOutEvent(QFocusEvent *) override;
  void wheelEvent(QWheelEvent *) override;

  int getCursorPos(int) const;
  int getMinCursorPos() const;
  int getMaxCursorPos() const;
  void setOffset(int, int);
  void constrainOffset();
  void updateMargins();

  void paintBackground(QPainter &);
  void paintText(QPainter &);
  void paintCursor(QPainter &);
  void paintSelection(QPainter &);

protected:
  void paintEvent(QPaintEvent *) override;
};

#endif
