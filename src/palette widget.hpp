//
//  palette widget.hpp
//  Animera
//
//  Created by Indi Kernick on 3/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef palette_widget_hpp
#define palette_widget_hpp

#include "image.hpp"
#include "palette span.hpp"
#include <QtWidgets/qscrollarea.h>

class ColorHandle;
class PaletteTableWidget;

class PaletteWidget final : public QScrollArea {
  Q_OBJECT

public:
  explicit PaletteWidget(QWidget *);
  
public Q_SLOTS:
  void initCanvas(Format);
  void setPalette(PaletteSpan);
  void attachIndex(int);
  
Q_SIGNALS:
  void shouldAttachColor(ColorHandle *);
  void shouldSetColor(QRgb);
  void shouldSetIndex(int);
  void paletteColorChanged();

private:
  PaletteTableWidget *table = nullptr;
};

#endif
