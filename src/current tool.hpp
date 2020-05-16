//
//  current tool.hpp
//  Animera
//
//  Created by Indiana Kernick on 24/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_current_tool_hpp
#define animera_current_tool_hpp

#include "tool.hpp"

class CurrentTool {
public:
  void setTool(Tool *);
  void setCel(Cel *);
  
  void mouseEnter(QPoint);
  void mouseEnter();
  void mouseLeave();
  void mouseDown(QPoint, ButtonType);
  void mouseMove(QPoint);
  void mouseUp(QPoint, ButtonType);
  void keyPress(Qt::Key);
  
private:
  Tool *tool = nullptr;
  Cel *cel = nullptr;
  QPoint lastPos = {-1, -1};
  ButtonType button = ButtonType::none;
  
  void attach();
  void detach();
};

#endif
