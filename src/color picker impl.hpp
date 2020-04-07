//
//  color picker impl.hpp
//  Animera
//
//  Created by Indiana Kernick on 10/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef color_picker_impl_hpp
#define color_picker_impl_hpp

#include <QtGui/qrgb.h>
#include <QtCore/qobject.h>
#include <QtWidgets/qgridlayout.h>

class ColorPickerImpl : public QObject {
  Q_OBJECT
  
public:
  virtual ~ColorPickerImpl();
  
  virtual void init(QWidget *) = 0;
  virtual void setupLayout(QGridLayout *) = 0;
  virtual void connectSignals() = 0;
  virtual void setColor(QRgb) = 0;

Q_SIGNALS:
  void colorChanged(QRgb);
  void shouldShowNorm(std::string_view);
};

#endif
