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
  // @TODO event processing
  // pressing another button while the mouse is still down will have no effect.
  // maybe assert that event.overlay is valid.
  return enabled ? tool->mouseDown(event) : ToolChanges::none;
}

ToolChanges CurrentTool::mouseMove(const ToolEvent &event) {
  assert(tool);
  // @TODO event processing
  // only post a mouseMove event if the mouse position has changed.
  // include the button that is being held while the mouse is moving.
  // this means that we can avoid duplicate processing in all the tools.
  // maybe assert that event.overlay is valid.
  return enabled ? tool->mouseMove(event) : ToolChanges::none;
}

ToolChanges CurrentTool::mouseUp(const ToolEvent &event) {
  assert(tool);
  // @TODO event processing
  // is it possible to get a mouse up event with a different position to
  // the previous mouse move event?
  // should only get a mouse up event for the drag button
  // maybe assert that event.overlay is valid
  return enabled ? tool->mouseUp(event) : ToolChanges::none;
}

void CurrentTool::attach() {
  enabled = cell ? tool->attachCell(cell) : false;
}

void CurrentTool::detach() {
  if (enabled) tool->detachCell();
}
