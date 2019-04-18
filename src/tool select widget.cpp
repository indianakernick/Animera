//
//  tools select widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool select widget.hpp"

#include "config.hpp"
#include <QtGui/qbitmap.h>
#include "tool widgets.hpp"
#include <QtGui/qpainter.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qabstractbutton.h>

template <typename T>
struct tag_t {};

template <typename T>
constexpr tag_t<T> tag{};

class ToolWidget final : public QAbstractButton {
  Q_OBJECT
  
public:
  template <typename WidgetClass>
  ToolWidget(
    ToolSelectWidget *tools,
    QWidget *parent,
    tag_t<WidgetClass>
  ) : QAbstractButton{parent},
      tools{tools},
      tool{std::make_unique<typename WidgetClass::impl>()},
      widget{std::make_unique<WidgetClass>()} {
    setCursor(Qt::PointingHandCursor);
    loadIcons(WidgetClass::icon_name);
    setToolTip(WidgetClass::tooltip);
    setCheckable(true);
    setAutoExclusive(true);
    setFixedSize(tool_button_size);
    setContentsMargins(0, 0, 0, 0);
    connect(this, &QAbstractButton::pressed, this, &ToolWidget::toolPressed);
  }
  
public Q_SLOTS:
  void toolPressed() {
    tools->changeTool(this, tool.get());
  }

private:
  ToolSelectWidget *tools;
  std::unique_ptr<Tool> tool;
  std::unique_ptr<QWidget> widget;
  // @TODO base bitmap is the same for all tool icons so you could share it
  QBitmap base;
  QBitmap shape;
  
  void loadIcons(const QString &name) {
    bool loaded = base.load(":/Tools/base.pbm");
    assert(loaded && name.data());
    base = base.scaled(tool_icon_size);
    loaded = shape.load(":/Tools/" + name + ".pbm");
    assert(loaded);
    shape = shape.scaled(tool_icon_size);
  }
  
  QColor baseColor() const {
    return isChecked() ? tool_base_enabled : tool_base_disabled;
  }
  QRegion maskRegion(const QBitmap &bitmap) const {
    QRegion region{bitmap};
    region.translate(tool_icon_pos);
    return region;
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.setClipRegion(maskRegion(base));
    painter.fillRect(QRect{tool_icon_pos, tool_icon_size}, baseColor());
    painter.setClipRegion(maskRegion(shape));
    painter.fillRect(QRect{tool_icon_pos, tool_icon_size}, tool_shape);
  }
};

ToolSelectWidget::ToolSelectWidget(QWidget *parent)
  : QScrollArea{parent}, box{new QWidget{this}} {
  setFixedWidth(tool_select_width);

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setStyleSheet("background-color: " + tool_select_background.name());
  
  QVBoxLayout *boxLayout = new QVBoxLayout{box};
  boxLayout->setSpacing(0);
  boxLayout->setContentsMargins(0, 0, 0, 0);
  box->setLayout(boxLayout);
  box->setContentsMargins(0, 0, 0, 0);
  setContentsMargins(0, 0, 0, 0);
  
  boxLayout->addStretch();
  makeToolWidget<BrushToolWidget>()->click();
  makeToolWidget<FloodFillToolWidget>();
  makeToolWidget<RectangleSelectToolWidget>();
  makeToolWidget<PolygonSelectToolWidget>();
  makeToolWidget<LineToolWidget>();
  makeToolWidget<StrokedCircleToolWidget>();
  makeToolWidget<FilledCircleToolWidget>();
  makeToolWidget<StrokedRectangleToolWidget>();
  makeToolWidget<FilledRectangleToolWidget>();
  makeToolWidget<TranslateToolWidget>();
  makeToolWidget<FlipToolWidget>();
  makeToolWidget<RotateToolWidget>();
  boxLayout->addStretch();
  
  setWidget(box);
  setAlignment(Qt::AlignVCenter);
  
  // @TODO remove
  colors.primary = qRgba(255, 0, 0, 255);
  colors.secondary = qRgba(0, 0, 255, 255);
  colors.erase = qRgba(0, 0, 0, 0);
}

void ToolSelectWidget::mouseLeave() {
  Q_EMIT updateStatusBar("");
}

void ToolSelectWidget::mouseDown(const QPoint pos, const ButtonType button, QImage *overlay) {
  // @TODO I think we need to move the status.clear() call somewhere else
  status.clear();
  emitModified(currTool.mouseDown({button, pos, colors, overlay, &status}));
  if (!status.empty()) Q_EMIT updateStatusBar(status.get());
}

void ToolSelectWidget::mouseMove(const QPoint pos, QImage *overlay) {
  status.clear();
  emitModified(currTool.mouseMove({{}, pos, colors, overlay, &status}));
  if (!status.empty()) Q_EMIT updateStatusBar(status.get());
}

void ToolSelectWidget::mouseUp(const QPoint pos, const ButtonType button, QImage *overlay) {
  status.clear();
  emitModified(currTool.mouseUp({button, pos, colors, overlay, &status}));
  if (!status.empty()) Q_EMIT updateStatusBar(status.get());
}

void ToolSelectWidget::keyPress(const Qt::Key key, QImage *overlay) {
  status.clear();
  emitModified(currTool.keyPress({key, colors, overlay, &status}));
  if (!status.empty()) Q_EMIT updateStatusBar(status.get());
}

void ToolSelectWidget::changeCell(Cell *cell) {
  currTool.changeCell(cell);
}

void ToolSelectWidget::changeTool(ToolWidget *widget, Tool *tool) {
  currWidget = widget;
  currTool.changeTool(tool);
}

template <typename WidgetClass>
ToolWidget *ToolSelectWidget::makeToolWidget() {
  ToolWidget *widget = new ToolWidget{this, box, tag<WidgetClass>};
  tools.push_back(widget);
  box->layout()->addWidget(widget);
  return widget;
}

void ToolSelectWidget::emitModified(const ToolChanges changes) {
  if (changes == ToolChanges::cell || changes == ToolChanges::cell_overlay) {
    Q_EMIT cellModified();
  } else if (changes == ToolChanges::overlay) {
    Q_EMIT overlayModified();
  }
}

#include "tool select widget.moc"
