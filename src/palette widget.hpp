//
//  palette widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef palette_widget_hpp
#define palette_widget_hpp

#include <QtWidgets/qscrollarea.h>

class ColorHandle;
class PaletteTableWidget;

class PaletteWidget final : public QScrollArea {
  Q_OBJECT

public:
  explicit PaletteWidget(QWidget *);
  
Q_SIGNALS:
  void attachColor(ColorHandle *);
  void setColor(QRgb);

private:
  PaletteTableWidget *table;
};

#endif
