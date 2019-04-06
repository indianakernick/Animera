//
//  undo.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "undo.hpp"

#include "config.hpp"

UndoStack::UndoStack() {
  stack.reserve(edit_undo_stack);
  top = -1;
}

bool UndoStack::empty() const {
  return stack.empty();
}

void UndoStack::clear() {
  stack.clear();
  top = -1;
}

void UndoStack::reset(Image img) {
  stack.clear();
  img.data.detach();
  stack.push_back(img);
  top = 0;
}

void UndoStack::modify(Image img) {
  assert(!stack.empty());
  stack.erase(stack.begin() + top + 1, stack.end());
  if (stack.size() >= edit_undo_stack) {
    const size_t oldImages = stack.size() - edit_undo_stack + 1;
    stack.erase(stack.begin(), stack.begin() + oldImages);
  }
  img.data.detach();
  stack.push_back(img);
  ++top;
}

UndoState UndoStack::undo() {
  assert(!stack.empty());
  if (top == 0) {
    return {stack.front(), false};
  }
  --top;
  return {stack[top], true};
}

UndoState UndoStack::redo() {
  assert(!stack.empty());
  if (top == stack.size() - 1) {
    return {stack.back(), false};
  }
  ++top;
  return {stack[top], true};
}
