//
//  tool.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool.hpp"

ToolChanges Tool::mouseDown(const ToolMouseEvent &) {
  return ToolChanges::none;
}

ToolChanges Tool::mouseMove(const ToolMouseEvent &) {
  return ToolChanges::none;
}

ToolChanges Tool::mouseUp(const ToolMouseEvent &) {
  return ToolChanges::none;
}

ToolChanges Tool::keyPress(const ToolKeyEvent &) {
  return ToolChanges::none;
}
