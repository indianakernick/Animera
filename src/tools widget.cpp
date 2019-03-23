//
//  tools widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tools widget.hpp"

#include "tool impls.hpp"
#include <QtGui/qpainter.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qabstractbutton.h>

class ToolWidget final : public QAbstractButton {
  Q_OBJECT

  static constexpr QSize icon_size {48, 48};
  static constexpr QSize button_size {52, 52};

public:
  ToolWidget(ToolsWidget *tools, QWidget *parent, std::unique_ptr<Tool> tool, const QString &name)
    : QAbstractButton{parent}, tools{tools}, tool{std::move(tool)} {
    loadIcons(name);
    setToolTip(name);
    setCheckable(true);
    setAutoExclusive(true);
    setFixedSize(button_size);
    setContentsMargins(0, 0, 0, 0);
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
    bool loaded = enabledIcon.load(":/Tools/" + name + " e.png");
    assert(loaded && name.data());
    enabledIcon = enabledIcon.scaled(icon_size);
    loaded = disabledIcon.load(":/Tools/" + name + " d.png");
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
  : QScrollArea{parent}, box{new QWidget{this}} {
  setFixedWidth(54);

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  
  QVBoxLayout *layout = new QVBoxLayout{box};
  layout->setSpacing(0);
  layout->setAlignment(Qt::AlignVCenter);
  layout->setContentsMargins(0, 0, 0, 0);
  box->setLayout(layout);
  box->setContentsMargins(0, 0, 0, 0);
  setContentsMargins(0, 0, 0, 0);
  
  layout->addStretch();
  makeToolWidget<BrushTool>("brush")->click();
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
  
  setWidget(box);
  
  // @TODO remove
  colors.primary = qRgba(255, 0, 0, 255);
  colors.secondary = qRgba(0, 0, 255, 255);
  colors.erase = qRgba(0, 0, 0, 0);
}

void ToolsWidget::mouseDown(const QPoint pos, const ButtonType button, QImage *overlay) {
  emitModified(currTool.mouseDown({button, pos, colors, overlay}));
}

void ToolsWidget::mouseMove(const QPoint pos, QImage *overlay) {
  emitModified(currTool.mouseMove({{}, pos, colors, overlay}));
}

void ToolsWidget::mouseUp(const QPoint pos, const ButtonType button, QImage *overlay) {
  emitModified(currTool.mouseUp({button, pos, colors, overlay}));
}

void ToolsWidget::keyPress(const Qt::Key key, QImage *overlay) {
  emitModified(currTool.keyPress({key, colors, overlay}));
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
  ToolWidget *widget = new ToolWidget{this, box, std::move(tool), name};
  tools.push_back(widget);
  box->layout()->addWidget(widget);
  return widget;
}

void ToolsWidget::emitModified(const ToolChanges changes) {
  if (changes == ToolChanges::cell || changes == ToolChanges::cell_overlay) {
    Q_EMIT cellModified();
  } else if (changes == ToolChanges::overlay) {
    Q_EMIT overlayModified();
  }
}

void ToolsWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  painter.fillRect(0, 0, width(), height(), QColor{127, 127, 127});
}

#include "tools widget.moc"
