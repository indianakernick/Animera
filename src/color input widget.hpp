//
//  color input widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 29/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_color_input_widget_hpp
#define animera_color_input_widget_hpp

#include "color convert.hpp"
#include <QtGui/qvalidator.h>
#include "text input widget.hpp"

class NumberValidator final : public QIntValidator {
public:
  NumberValidator(QWidget *, IntRange, bool);
  
  void fixup(QString &) const override;
  State validate(QString &, int &) const override;
  void updateValidValue(const QString &);

private:
  QString lastValidValue;
  bool skipZero;
};

class HexValidator final : public QValidator {
public:
  explicit HexValidator(QWidget *);
  
  void fixup(QString &) const override;
  State validate(QString &, int &) const override;
  void updateValidValue(const QString &);

private:
  QString lastValidValue;
};

class NumberInputWidget final : public TextInputWidget {
  Q_OBJECT
public:
  NumberInputWidget(QWidget *, WidgetRect, IntRange, bool = false);
  
  int value() const;
  
Q_SIGNALS:
  void valueChanged(int);

public Q_SLOTS:
  void setValue(int);

private Q_SLOTS:
  void textChanged();
  void newValidValue();

private:
  NumberValidator boxValidator;
  int val;
  bool skipZero;
  
  void keyPressEvent(QKeyEvent *) override;
};

class HexInputWidget final : public TextInputWidget {
  Q_OBJECT
public:
  HexInputWidget(QWidget *, WidgetRect, RGB, int);

Q_SIGNALS:
  void rgbaChanged(RGB, int);

public Q_SLOTS:
  void setRgba(RGB, int);

private Q_SLOTS:
  void textChanged();
  void newValidValue();

private:
  HexValidator boxValidator;
  RGB rgb;
  int alpha;
};

#endif
