//
//  undo.hpp
//  Animera
//
//  Created by Indiana Kernick on 18/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_undo_hpp
#define animera_undo_hpp

#include <vector>
#include "cel.hpp"

struct UndoState {
  Cel cel;
  bool undid;
};

class UndoStack {
public:
  UndoStack();

  bool empty() const;
  void clear();
  void reset(Cel);
  void modify(Cel);
  UndoState undo();
  UndoState redo();
  
private:
  std::vector<Cel> stack;
  std::size_t top;
};

#endif
