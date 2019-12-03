//
//  current tool.cpp
//  Animera
//
//  Created by Indi Kernick on 24/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "current tool.hpp"

void CurrentTool::setTool(Tool *newTool) {
  assert(newTool);
  detach();
  tool = newTool;
  attach();
}

void CurrentTool::setCell(Cell *newCell) {
  assert(newCell);
  detach();
  cell = newCell;
  attach();
}

void CurrentTool::mouseEnter(const QPoint pos) {
  assert(tool);
  if (button == ButtonType::none) {
    lastPos = pos;
    tool->mouseMove({lastPos, lastPos, ButtonType::none});
  }
}

void CurrentTool::mouseEnter() {
  assert(tool);
  if (button == ButtonType::none) {
    tool->mouseMove({lastPos, lastPos, ButtonType::none});
  }
}

void CurrentTool::mouseLeave() {
  assert(tool);
  tool->mouseLeave({lastPos, button});
}

void CurrentTool::mouseDown(const QPoint ePos, const ButtonType eButton) {
  assert(tool);
  if (button == ButtonType::none) {
    if (ePos != lastPos) {
      // I'm pretty sure this never happens but now we can safely assume so
      tool->mouseMove({lastPos, ePos, ButtonType::none});
    }
    lastPos = ePos;
    button = eButton;
    tool->mouseDown({ePos, lastPos, eButton});
  }
}

void CurrentTool::mouseMove(const QPoint ePos) {
  assert(tool);
  if (ePos != lastPos) {
    tool->mouseMove({ePos, lastPos, button});
    lastPos = ePos;
  }
}

void CurrentTool::mouseUp(const QPoint ePos, const ButtonType eButton) {
  assert(tool);
  if (eButton == button) {
    if (ePos != lastPos) {
      // I'm pretty sure this never happens but now we can safely assume so
      tool->mouseMove({ePos, lastPos, button});
    }
    lastPos = ePos;
    button = ButtonType::none;
    tool->mouseUp({ePos, lastPos, eButton});
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
