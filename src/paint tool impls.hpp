//
//  paint tool impls.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 19/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef paint_tool_impls_hpp
#define paint_tool_impls_hpp

#include "tool.hpp"
#include <QtGui/qpen.h>

class SourceCell;

class BrushTool : public Tool {
public:
  BrushTool();

  bool attachCell(Cell *) override;
  ToolChanges mouseDown(const ToolEvent &) override;
  ToolChanges mouseMove(const ToolEvent &) override;
  ToolChanges mouseUp(const ToolEvent &) override;

  void setDiameter(int);
  int getDiameter() const;
  
  static constexpr int min_diameter = 1;
  static constexpr int max_diameter = 64;

private:
  QPoint lastPos = {-1, -1};
  SourceCell *source = nullptr;
  ButtonType button = ButtonType::none;
  QPen pen;
  
  QPen overlayPen() const;
  void clearOverlay(QImage *);
  void drawOverlay(QImage *, QPoint);
};

#endif
