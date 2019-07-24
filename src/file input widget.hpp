//
//  file input widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef file_input_widget_hpp
#define file_input_widget_hpp

#include <QtWidgets/qwidget.h>

class TextInputWidget;
class FileInputButton;

class FileInputWidget final : public QWidget {
public:
  FileInputWidget(QWidget *, int);

private:
  TextInputWidget *text;
  FileInputButton *icon;

  void initText();
  void setTextFromDialog();
};

#endif
