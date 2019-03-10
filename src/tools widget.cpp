//
//  tools widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tools widget.hpp"

#include "tool impls.hpp"
#include <QtCore/qdir.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qradiobutton.h>

#include <iostream>

class ToolWidget final : public QAbstractButton {
  Q_OBJECT

public:
  // layout is wrong if using a QRadioButton
  ToolWidget(ToolsWidget *tools, std::unique_ptr<Tool> tool, const QString &iconPath)
    : QAbstractButton{tools}, tools{tools}, tool{std::move(tool)}, icon{iconPath} {
    icon = icon.scaled(48, 48);
    connect(this, &QAbstractButton::pressed, this, &ToolWidget::toolPressed);
    setFixedSize(52, 52);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setContentsMargins(0, 0, 0, 0);
  }
  
public Q_SLOTS:
  void toolPressed() {
    tools->changeTool(tool.get());
  }

private:
  ToolsWidget *tools;
  std::unique_ptr<Tool> tool;
  QPixmap icon;
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.drawPixmap(QRect{
      (width() - icon.width()) / 2,
      (height() - icon.height()) / 2,
      icon.width(),
      icon.height()
    }, icon);
  }
};

ToolsWidget::ToolsWidget(QWidget *parent)
  : QGroupBox{parent} {
  setFixedWidth(52);
  
  QVBoxLayout *layout = new QVBoxLayout{this};
  layout->setSpacing(0);
  layout->setAlignment(Qt::AlignTop);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  setContentsMargins(0, 0, 0, 0);
  
  makeToolWidget<BrushTool>("brush");
  makeToolWidget<FloodFillTool>("fill");
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
  
  layout->addStretch();
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
  const QString iconPath = "/Users/indikernick/Library/Developer/Xcode/DerivedData/Pixel_2-gqoblrlhvynmicgniivandqktune/Build/Products/Debug/Pixel 2.app/Contents/Resources/" + iconName + " tool.png";
  ToolWidget *widget = new ToolWidget{this, std::move(tool), iconPath};
  tools.push_back(widget);
  layout()->addWidget(widget);
}

void ToolsWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.fillRect(0, 0, width(), height(), QColor{127, 127, 127});
}

#include "tools widget.moc"
