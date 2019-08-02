//
//  undo.hpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef undo_hpp
#define undo_hpp

#include <vector>
#include "image.hpp"

struct UndoState {
  QImage img;
  bool undid;
};

class UndoStack {
public:
  UndoStack();

  bool empty() const;
  void clear();
  void reset(QImage);
  void modify(QImage);
  UndoState undo();
  UndoState redo();
  
private:
  std::vector<QImage> stack;
  size_t top;
};

#endif
