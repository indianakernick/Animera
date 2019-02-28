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

ToolChanges CurrentTool::mouseDown(const ToolMouseEvent &event) {
  assert(tool);
  assert(event.overlay);
  if (button == ButtonType::none) {
    button = event.button;
    lastPos = event.pos;
    return enabled ? tool->mouseDown(event) : ToolChanges::none;
  } else {
    return ToolChanges::none;
  }
}

ToolChanges CurrentTool::mouseMove(const ToolMouseEvent &event) {
  assert(tool);
  assert(event.overlay);
  if (event.pos == lastPos) {
    return ToolChanges::none;
  } else {
    lastPos = event.pos;
    const ToolMouseEvent newEvent{button, event.pos, event.colors, event.overlay};
    return enabled ? tool->mouseMove(newEvent) : ToolChanges::none;
  }
}

ToolChanges CurrentTool::mouseUp(const ToolMouseEvent &event) {
  assert(tool);
  assert(event.overlay);
  if (event.button == button) {
    button = ButtonType::none;
    lastPos = event.pos;
    return enabled ? tool->mouseUp(event) : ToolChanges::none;
  } else {
    return ToolChanges::none;
  }
}

ToolChanges CurrentTool::keyPress(const ToolKeyEvent &event) {
  assert(tool);
  assert(event.overlay);
  return enabled ? tool->keyPress(event) : ToolChanges::none;
}

void CurrentTool::attach() {
  enabled = cell ? tool->attachCell(cell) : false;
}

void CurrentTool::detach() {
  if (enabled) tool->detachCell();
}
