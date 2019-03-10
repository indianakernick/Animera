//
//  tools widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef tools_widget_hpp
#define tools_widget_hpp

#include "current tool.hpp"
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qboxlayout.h>

class ToolWidget;

class ToolsWidget final : public QGroupBox {
  Q_OBJECT

  friend class ToolWidget;

public:
  explicit ToolsWidget(QWidget *);
  
public Q_SLOTS:
  // changes the cell that this
  void changeCell(Cell *);
  
private:
  CurrentTool currTool;
  std::vector<ToolWidget *> tools;
  
  void changeTool(Tool *);
  template <typename ToolClass>
  void makeToolWidget(const QString &);
  
  void paintEvent(QPaintEvent *) override;
};

#endif
