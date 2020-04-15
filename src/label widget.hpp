//
//  label widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 29/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_label_widget_hpp
#define animera_label_widget_hpp

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

template <std::size_t Size>
LabelWidget *makeLabel(QWidget *parent, const char (&text)[Size]) {
  return new LabelWidget{parent, textBoxRect(Size - 1), text};
}

template <std::size_t Size>
LabelWidget *makeLabel(QWidget *parent, const int chars, const char (&text)[Size]) {
  assert(int{Size} - 1 <= chars);
  return new LabelWidget{parent, textBoxRect(chars), text};
}

inline LabelWidget *makeLabel(QWidget *parent, const QString &text) {
  return new LabelWidget{parent, textBoxRect(text.size()), text};
}

enum class WrapMode {
  none,
  word,
  anywhere,
  word_or_anywhere
};

QSize wrapToWidth(QString &, int, WrapMode);

#endif
