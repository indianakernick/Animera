//
//  text input widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 29/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_text_input_widget_hpp
#define animera_text_input_widget_hpp

#include "widget rect.hpp"
#include <QtCore/qtimer.h>
#include <QtWidgets/qlineedit.h>

class TextInputWidget : public QLineEdit {
  Q_OBJECT
  
public:
  TextInputWidget(QWidget *, const WidgetRect &);

  void setText(const QString &);

private Q_SLOTS:
  void blink();
  void showCursor();

private:
  WidgetRect rect;
  int offset = 0;
  QTimer cursorBlinkTimer;
  bool cursorBlinkStatus = true;

  int getTextWidth(int = 0, int = -1) const;
  int getAlignOffset() const;
  int getCursorPos(int) const;
  int getMinCursorPos() const;
  int getMaxCursorPos() const;
  int getMinOffset() const;
  
  void setOffset(int, int);
  void constrainOffset();
  void updateMargins();

  void paintBackground(QPainter &);
  void paintText(QPainter &);
  void paintCursor(QPainter &);
  void paintSelection(QPainter &);

protected:
  void keyPressEvent(QKeyEvent *) override;
  void focusInEvent(QFocusEvent *) override;
  void focusOutEvent(QFocusEvent *) override;
  void wheelEvent(QWheelEvent *) override;
  void paintEvent(QPaintEvent *) override;
};

#endif
