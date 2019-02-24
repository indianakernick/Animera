//
//  current tool.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "current tool.hpp"

void CurrentTool::changeTool(Tool *newTool) {
  assert(newTool);
  detach();
  tool = newTool;
  attach();
}

void CurrentTool::changeCell(Cell *newCell) {
  detach();
  cell = newCell;
  attach();
}

ToolChanges CurrentTool::mouseDown(const ToolEvent &event) {
  assert(tool);
  return enabled ? tool->mouseDown(event) : ToolChanges::none;
}

ToolChanges CurrentTool::mouseMove(const ToolEvent &event) {
  assert(tool);
  return enabled ? tool->mouseMove(event) : ToolChanges::none;
}

ToolChanges CurrentTool::mouseUp(const ToolEvent &event) {
  assert(tool);
  return enabled ? tool->mouseUp(event) : ToolChanges::none;
}

void CurrentTool::attach() {
  enabled = cell ? tool->attachCell(cell) : false;
}

void CurrentTool::detach() {
  if (enabled) tool->detachCell();
}
