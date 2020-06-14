//
//  palette widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 3/6/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_palette_widget_hpp
#define animera_palette_widget_hpp

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
  void attachIndex(PixelIndex);
  void updatePalette();
  
Q_SIGNALS:
  void shouldAttachColor(ColorHandle *);
  void shouldSetColor(PixelVar);
  void shouldSetIndex(PixelIndex);
  void paletteColorChanged();
  void shouldShowNorm(std::string_view);

private:
  PaletteTableWidget *table = nullptr;
};

#endif
