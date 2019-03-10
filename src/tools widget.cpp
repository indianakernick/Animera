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

class ToolWidget final : public QAbstractButton {
  Q_OBJECT

  static constexpr QSize icon_size {48, 48};
  static constexpr QSize button_size {52, 52};
  // @TODO use Qt resource system
  static inline const QString resource_path = "/Users/indikernick/Library/Developer/Xcode/DerivedData/Pixel_2-gqoblrlhvynmicgniivandqktune/Build/Products/Debug/Pixel 2.app/Contents/Resources/";

public:
  ToolWidget(ToolsWidget *tools, std::unique_ptr<Tool> tool, const QString &name)
    : QAbstractButton{tools}, tools{tools}, tool{std::move(tool)} {
    loadIcons(name);
    setToolTip(name);
    setCheckable(true);
    setAutoExclusive(true);
    setFixedSize(button_size);
    setContentsMargins(0, 0, 0, 0);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(this, &QAbstractButton::pressed, this, &ToolWidget::toolPressed);
  }
  
public Q_SLOTS:
  void toolPressed() {
    tools->changeTool(tool.get());
  }

private:
  ToolsWidget *tools;
  std::unique_ptr<Tool> tool;
  QPixmap enabledIcon;
  QPixmap disabledIcon;
  
  void loadIcons(const QString &name) {
    bool loaded = enabledIcon.load(resource_path + name + " tool enabled.png");
    assert(loaded && name.data());
    enabledIcon = enabledIcon.scaled(icon_size);
    loaded = disabledIcon.load(resource_path + name + " tool disabled.png");
    assert(loaded);
    disabledIcon = disabledIcon.scaled(icon_size);
  }
  
  void paintEvent(QPaintEvent *) override {
    if (isChecked()) {
      drawIcon(enabledIcon);
    } else {
      drawIcon(disabledIcon);
    }
  }
  
  void drawIcon(const QPixmap &icon) {
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
  layout->setAlignment(Qt::AlignVCenter);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);
  setContentsMargins(0, 0, 0, 0);
  
  layout->addStretch();
  makeToolWidget<BrushTool>("brush")->toolPressed();
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
  layout->addStretch();
}

void ToolsWidget::changeCell(Cell *cell) {
  currTool.changeCell(cell);
}

void ToolsWidget::changeTool(Tool *tool) {
  currTool.changeTool(tool);
}

template <typename ToolClass>
ToolWidget *ToolsWidget::makeToolWidget(const QString &name) {
  std::unique_ptr<Tool> tool = std::make_unique<ToolClass>();
  ToolWidget *widget = new ToolWidget{this, std::move(tool), name};
  tools.push_back(widget);
  layout()->addWidget(widget);
  return widget;
}

void ToolsWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.fillRect(0, 0, width(), height(), QColor{127, 127, 127});
}

#include "tools widget.moc"
