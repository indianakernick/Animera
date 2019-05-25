//
//  color input widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 29/4/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef color_input_widget_hpp
#define color_input_widget_hpp

#include "color convert.hpp"
#include <QtGui/qvalidator.h>
#include "text input widget.hpp"

class NumberValidator final : public QIntValidator {
public:
  NumberValidator(QWidget *, int);
  
  void fixup(QString &) const override;
  void updateValidValue(const QString &);

private:
  QString lastValidValue;
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
  NumberInputWidget(QWidget *, int, int);

Q_SIGNALS:
  void valueChanged(int);

public Q_SLOTS:
  void changeValue(int);

private Q_SLOTS:
  void textChanged();
  void newValidValue();

private:
  NumberValidator boxValidator;
  int value;
  
  void keyPressEvent(QKeyEvent *) override;
};

class HexInputWidget final : public TextInputWidget {
  Q_OBJECT
public:
  HexInputWidget(QWidget *, RGB, int);

Q_SIGNALS:
  void rgbaChanged(RGB, int);

public Q_SLOTS:
  void changeRgba(RGB, int);

private Q_SLOTS:
  void textChanged();
  void newValidValue();

private:
  HexValidator boxValidator;
  RGB rgb;
  int alpha;
};

#endif
