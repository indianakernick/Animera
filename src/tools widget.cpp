//
//  tools widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tools widget.hpp"

#include "tool impls.hpp"
#include <QtWidgets/qradiobutton.h>

class ToolWidget final : public QRadioButton {
  Q_OBJECT

public:
  ToolWidget(ToolsWidget *tools, std::unique_ptr<Tool> tool, const QIcon &icon)
    : QRadioButton{tools}, tools{tools}, tool{std::move(tool)} {
    connect(this, &QAbstractButton::pressed, this, &ToolWidget::toolPressed);
    setIcon(icon);
  }
  
public Q_SLOTS:
  void toolPressed() {
    tools->changeTool(tool.get());
  }

private:
  ToolsWidget *tools;
  std::unique_ptr<Tool> tool;
};

ToolsWidget::ToolsWidget(QWidget *parent)
  : QGroupBox{parent} {
  QVBoxLayout *layout = new QVBoxLayout{this};
  makeToolWidget<BrushTool>("brush");
  makeToolWidget<FloodFillTool>("flood fill");
  makeToolWidget<RectangleSelectTool>("rectangle select");
  makeToolWidget<MaskSelectTool>("mask select");
  makeToolWidget<LineTool>("line");
  makeToolWidget<StrokedCircleTool>("stroked circle");
  makeToolWidget<FilledCircleTool>("filled circle");
  makeToolWidget<StrokedRectangleTool>("stroked rectangle");
  makeToolWidget<FilledRectangleTool>("filled rectangle");
  makeToolWidget<TranslateTool>("translate");
  makeToolWidget<FlipTool>("flip");
  makeToolWidget<RotateTool>("rotate");
  setLayout(layout);
}

void ToolsWidget::changeCell(Cell *cell) {
  currTool.changeCell(cell);
}

void ToolsWidget::changeTool(Tool *tool) {
  currTool.changeTool(tool);
}

template <typename ToolClass>
void ToolsWidget::makeToolWidget(const QString &iconName) {
  std::unique_ptr<Tool> tool = std::make_unique<ToolClass>();
  QIcon icon{iconName + " tool.png"};
  ToolWidget *widget = new ToolWidget{this, std::move(tool), icon};
  tools.push_back(widget);
  layout()->addWidget(widget);
}

#include "tools widget.moc"
