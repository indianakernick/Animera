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

void CurrentTool::mouseLeave() {
  assert(tool);
  tool->mouseLeave({button});
}

void CurrentTool::mouseDown(const QPoint ePos, const ButtonType eButton) {
  assert(tool);
  if (button == ButtonType::none) {
    button = eButton;
    lastPos = ePos;
    tool->mouseDown({ePos, eButton});
  }
}

void CurrentTool::mouseMove(const QPoint ePos) {
  assert(tool);
  if (ePos != lastPos) {
    lastPos = ePos;
    tool->mouseMove({ePos, button});
  }
}

void CurrentTool::mouseUp(const QPoint ePos, const ButtonType eButton) {
  assert(tool);
  if (eButton == button) {
    button = ButtonType::none;
    lastPos = ePos;
    tool->mouseUp({ePos, eButton});
  }
}

void CurrentTool::keyPress(const Qt::Key eKey) {
  assert(tool);
  tool->keyPress({eKey});
}

void CurrentTool::attach() {
  if (cell) tool->attachCell();
}

void CurrentTool::detach() {
  if (cell) tool->detachCell();
}
