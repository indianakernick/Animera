//
//  label widget.hpp
//  Animera
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef label_widget_hpp
#define label_widget_hpp

#include "config.hpp"
#include <QtWidgets/qwidget.h>

class LabelWidget final : public QWidget {
public:
  LabelWidget(QWidget *, WidgetRect, const QString &);

  void setText(const QString &);

private:
  QString text;
  WidgetRect rect;

  void paintEvent(QPaintEvent *) override;
};

template <size_t Size>
LabelWidget *makeLabel(QWidget *parent, const char (&text)[Size]) {
  return new LabelWidget{parent, textBoxRect(Size - 1), text};
}

template <size_t Size>
LabelWidget *makeLabel(QWidget *parent, const int chars, const char (&text)[Size]) {
  assert(int{Size} - 1 <= chars);
  return new LabelWidget{parent, textBoxRect(chars), text};
}

inline LabelWidget *makeLabel(QWidget *parent, const QString &text) {
  return new LabelWidget{parent, textBoxRect(text.size()), text};
}

#endif
