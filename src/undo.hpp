//
//  undo.hpp
//  Animera
//
//  Created by Indiana Kernick on 18/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef undo_hpp
#define undo_hpp

#include <vector>
#include "cell.hpp"

struct UndoState {
  Cell cell;
  bool undid;
};

class UndoStack {
public:
  UndoStack();

  bool empty() const;
  void clear();
  void reset(Cell);
  void modify(Cell);
  UndoState undo();
  UndoState redo();
  
private:
  std::vector<Cell> stack;
  std::size_t top;
};

#endif
