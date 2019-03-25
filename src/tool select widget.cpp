//
//  tools select widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool select widget.hpp"

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

  static constexpr QSize icon_size {48, 48};
  static constexpr QSize button_size {52, 52};

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
    loadIcons(WidgetClass::icon_name);
    setToolTip(WidgetClass::tooltip);
    setCheckable(true);
    setAutoExclusive(true);
    setFixedSize(button_size);
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

ToolSelectWidget::ToolSelectWidget(QWidget *parent)
  : QScrollArea{parent}, box{new QWidget{this}} {
  setFixedWidth(54);

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setStyleSheet("background-color: " + QColor{63, 63, 127}.name());
  
  QVBoxLayout *layout = new QVBoxLayout{box};
  layout->setSpacing(0);
  layout->setAlignment(Qt::AlignVCenter);
  layout->setContentsMargins(0, 0, 0, 0);
  box->setLayout(layout);
  box->setContentsMargins(0, 0, 0, 0);
  setContentsMargins(0, 0, 0, 0);
  
  layout->addStretch();
  makeToolWidget<BrushToolWidget>()->click();
  makeToolWidget<FloodFillToolWidget>();
  makeToolWidget<RectangleSelectToolWidget>();
  makeToolWidget<MaskSelectToolWidget>();
  makeToolWidget<LineToolWidget>();
  makeToolWidget<StrokedCircleToolWidget>();
  makeToolWidget<FilledCircleToolWidget>();
  makeToolWidget<StrokedRectangleToolWidget>();
  makeToolWidget<FilledRectangleToolWidget>();
  makeToolWidget<TranslateToolWidget>();
  makeToolWidget<FlipToolWidget>();
  makeToolWidget<RotateToolWidget>();
  layout->addStretch();
  
  setWidget(box);
  
  // @TODO remove
  colors.primary = qRgba(255, 0, 0, 255);
  colors.secondary = qRgba(0, 0, 255, 255);
  colors.erase = qRgba(0, 0, 0, 0);
}

void ToolSelectWidget::mouseLeave() {
  Q_EMIT updateStatusBar("");
}

void ToolSelectWidget::mouseDown(const QPoint pos, const ButtonType button, QImage *overlay) {
  std::string status;
  emitModified(currTool.mouseDown({button, pos, colors, overlay, &status}));
  if (!status.empty()) Q_EMIT updateStatusBar(status);
}

void ToolSelectWidget::mouseMove(const QPoint pos, QImage *overlay) {
  std::string status;
  emitModified(currTool.mouseMove({{}, pos, colors, overlay, &status}));
  if (!status.empty()) Q_EMIT updateStatusBar(status);
}

void ToolSelectWidget::mouseUp(const QPoint pos, const ButtonType button, QImage *overlay) {
  std::string status;
  emitModified(currTool.mouseUp({button, pos, colors, overlay, &status}));
  if (!status.empty()) Q_EMIT updateStatusBar(status);
}

void ToolSelectWidget::keyPress(const Qt::Key key, QImage *overlay) {
  std::string status;
  emitModified(currTool.keyPress({key, colors, overlay, &status}));
  if (!status.empty()) Q_EMIT updateStatusBar(status);
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
