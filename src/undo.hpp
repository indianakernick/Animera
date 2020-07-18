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
  CelImage cel;
  bool undid;
};

class UndoStack {
public:
  UndoStack();

  bool empty() const;
  void clear();
  void reset(CelImage);
  void modify(CelImage);
  UndoState undo();
  UndoState redo();
  
private:
  std::vector<CelImage> stack;
  std::size_t top;
};

#endif
