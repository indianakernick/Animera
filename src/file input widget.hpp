//
//  file input widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef file_input_widget_hpp
#define file_input_widget_hpp

#include "text input widget.hpp"

class FileInputWidget final : public TextInputWidget {
public:
  FileInputWidget(QWidget *, int);

private:
  TextBoxRect rect;
  QPixmap arrow;

  void paintEvent(QPaintEvent *) override;
};

#endif
