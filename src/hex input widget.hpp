//
//  hex input widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 29/4/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_hex_input_widget_hpp
#define animera_hex_input_widget_hpp

#include "color convert.hpp"
#include <QtGui/qvalidator.h>
#include "text input widget.hpp"

class HexValidator final : public QValidator {
public:
  explicit HexValidator(QWidget *);
  
  void fixup(QString &) const override;
  State validate(QString &, int &) const override;
  void updateValidValue(const QString &);

private:
  QString lastValidValue;
};

class HexInputWidget final : public TextInputWidget {
  Q_OBJECT
  
public:
  HexInputWidget(QWidget *, const WidgetRect &, RGB, int);

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
