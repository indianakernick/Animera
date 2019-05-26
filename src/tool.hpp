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
#include "status msg.hpp"

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

struct ToolLeaveEvent {
  ButtonType button;
  QImage *overlay = nullptr;
  StatusMsg *status = nullptr;
};

struct ToolMouseEvent {
  ButtonType button;
  QPoint pos;
  ToolColors colors;
  QImage *overlay = nullptr; // @TODO does this really need to be a pointer?
  StatusMsg *status = nullptr;
};

struct ToolKeyEvent {
  Qt::Key key;
  ToolColors colors;
  QImage *overlay = nullptr;
  StatusMsg *status = nullptr;
};

class Tool {
public:
  virtual ~Tool() = default;
  
  virtual bool attachCell(Cell *) = 0;
  virtual void detachCell() = 0;
  virtual ToolChanges mouseLeave(const ToolLeaveEvent &);
  virtual ToolChanges mouseDown(const ToolMouseEvent &);
  virtual ToolChanges mouseMove(const ToolMouseEvent &);
  virtual ToolChanges mouseUp(const ToolMouseEvent &);
  virtual ToolChanges keyPress(const ToolKeyEvent &);
};

#endif
