//
//  number input widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 18/6/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_number_input_widget_hpp
#define animera_number_input_widget_hpp

#include "int range.hpp"
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

class NumberInputWidget final : public TextInputWidget {
  Q_OBJECT
  
public:
  NumberInputWidget(QWidget *, const WidgetRect &, IntRange, bool = false);
  
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

#endif
