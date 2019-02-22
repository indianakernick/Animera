//
//  tool.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef tool_hpp
#define tool_hpp

#include "image.hpp"

class Cell;

enum class ButtonType {
  none,
  primary,
  secondary,
  erase
};

enum class ToolChanges {
  none,
  cell,
  overlay,
  cell_overlay
};

struct ToolColors {
  QRgb primary;
  QRgb secondary;
  QRgb erase;
};

struct ToolEvent {
  ButtonType type;
  QPoint pos;
  ToolColors colors;
  QImage *overlay = nullptr;
};

constexpr int overlay_alpha = 127;
constexpr int overlay_gray = 127;
constexpr QRgb overlay_color = qRgba(
  overlay_gray, overlay_gray, overlay_gray, overlay_alpha
);

class Tool {
public:
  virtual ~Tool();
  
  virtual bool attachCell(Cell *) = 0;
  virtual ToolChanges mouseDown(const ToolEvent &) = 0;
  virtual ToolChanges mouseMove(const ToolEvent &) = 0;
  virtual ToolChanges mouseUp(const ToolEvent &) = 0;
};

#endif
