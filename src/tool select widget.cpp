//
//  tools select widget.cpp
//  Animera
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "tool select widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include <QtGui/qbitmap.h>
#include "tool widgets.hpp"
#include <QtGui/qpainter.h>
#include "widget painting.hpp"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollbar.h>
#include "radio button widget.hpp"

template <typename T>
struct tag_t {};

template <typename T>
constexpr tag_t<T> tag{};

class ToolWidget final : public RadioButtonWidget {
  Q_OBJECT
  
public:
  template <typename WidgetClass>
  ToolWidget(QWidget *parent, const ToolCtx *ctx, tag_t<WidgetClass>)
    : RadioButtonWidget{parent},
      tool{std::make_unique<typename WidgetClass::impl>()},
      widget{std::make_unique<WidgetClass>()} {
    tool->setCtx(ctx);
    loadIcons(WidgetClass::icon_name);
    setToolTip(WidgetClass::tooltip);
    setFixedSize(tool_button_size);
    CONNECT(this, toggled, this, changeTool);
  }

Q_SIGNALS:
  void shouldChangeTool(ToolWidget *, Tool *);

private Q_SLOTS:
  void changeTool(const bool checked) {
    if (checked) {
      Q_EMIT shouldChangeTool(this, tool.get());
    }
  }

private:
  std::unique_ptr<Tool> tool;
  std::unique_ptr<QWidget> widget;
  QPixmap enabledIcon;
  QPixmap disabledIcon;
  
  void loadIcons(const QString &name) {
    QBitmap base = QBitmap{":/Tools/base.png"}.scaled(tool_icon_size);
    QBitmap shape = QBitmap{":/Tools/" + name + ".png"}.scaled(tool_icon_size);
    enabledIcon = bakeColoredBitmaps(base, shape, tool_base_enabled, tool_shape_enabled);
    disabledIcon = bakeColoredBitmaps(base, shape, tool_base_disabled, tool_shape_disabled);
    setMask(QRegion{base}.translated(tool_icon_pos));
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.drawPixmap(tool_icon_pos, isChecked() ? enabledIcon : disabledIcon);
  }
};

ToolSelectWidget::ToolSelectWidget(QWidget *parent)
  : QScrollArea{parent}, box{new QWidget{this}} {
  setFixedWidth(tool_select_width);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setStyleSheet("background-color: " + tool_select_background.name());
  setFrameShape(NoFrame);
  createTools();
  setupLayout();
  connectSignals();
  setWidget(box);
  setAlignment(Qt::AlignVCenter);
}

void ToolSelectWidget::mouseEnter(const QPoint pos) {
  currTool.mouseEnter(pos);
  mouseIn = true;
}

void ToolSelectWidget::mouseLeave() {
  currTool.mouseLeave();
  mouseIn = false;
}

void ToolSelectWidget::mouseDown(const QPoint pos, const ButtonType button) {
  currTool.mouseDown(pos, button);
}

void ToolSelectWidget::mouseMove(const QPoint pos) {
  currTool.mouseMove(pos);
  mouseIn = true;
}

void ToolSelectWidget::mouseUp(const QPoint pos, const ButtonType button) {
  currTool.mouseUp(pos, button);
}

void ToolSelectWidget::keyPress(const Qt::Key key) {
  if (key == key_tool_up) {
    for (auto t = tools.begin(); t != std::prev(tools.end()); ++t) {
      if (*std::next(t) == currWidget) {
        (*t)->click();
        ensureVisible(*t);
        break;
      }
    }
  } else if (key == key_tool_down) {
    for (auto t = std::next(tools.begin()); t != tools.end(); ++t) {
      if (*std::prev(t) == currWidget) {
        (*t)->click();
        ensureVisible(*t);
        break;
      }
    }
  } else {
    currTool.keyPress(key);
  }
}

void ToolSelectWidget::setOverlay(QImage *overlay) {
  ctx.overlay = overlay;
}

void ToolSelectWidget::setCell(Cell *cell) {
  ctx.cell = cell;
  if (mouseIn) currTool.mouseLeave();
  currTool.setCell(cell);
  if (mouseIn) currTool.mouseEnter();
}

void ToolSelectWidget::setColors(const ToolColors colors) {
  ctx.colors = colors;
}

void ToolSelectWidget::setPalette(const PaletteCSpan palette) {
  ctx.palette = palette;
}

void ToolSelectWidget::initCanvas(const Format format, const QSize size) {
  ctx.format = format;
  ctx.size = size;
}

void ToolSelectWidget::setTool(ToolWidget *widget, Tool *tool) {
  currWidget = widget;
  if (mouseIn) currTool.mouseLeave();
  currTool.setTool(tool);
  if (mouseIn) currTool.mouseEnter();
}

void ToolSelectWidget::lockTool() {
  for (ToolWidget *tool : tools) {
    tool->setEnabled(false);
  }
}

void ToolSelectWidget::unlockTool() {
  for (ToolWidget *tool : tools) {
    tool->setEnabled(true);
  }
}

void ToolSelectWidget::ensureVisible(ToolWidget *tool) {
  // TODO: Qt bug
  // https://bugreports.qt.io/browse/QTBUG-80093
  QScrollBar *vbar = verticalScrollBar();
  const int before = vbar->value();
  ensureWidgetVisible(tool, 0, 0);
  const int now = vbar->value();
  if (now > before) {
    vbar->setValue(now + 1);
  }
  repaint();
}

template <typename WidgetClass>
void ToolSelectWidget::pushToolWidget() {
  tools.push_back(new ToolWidget{box, &ctx, tag<WidgetClass>});
}

void ToolSelectWidget::createTools() {
  pushToolWidget<BrushToolWidget>();
  pushToolWidget<FloodFillToolWidget>();
  pushToolWidget<RectangleSelectToolWidget>();
  pushToolWidget<PolygonSelectToolWidget>();
  pushToolWidget<WandSelectToolWidget>();
  pushToolWidget<LineToolWidget>();
  pushToolWidget<StrokedCircleToolWidget>();
  pushToolWidget<FilledCircleToolWidget>();
  pushToolWidget<StrokedRectangleToolWidget>();
  pushToolWidget<FilledRectangleToolWidget>();
  pushToolWidget<LinearGradientToolWidget>();
  pushToolWidget<TranslateToolWidget>();
  pushToolWidget<FlipToolWidget>();
  pushToolWidget<RotateToolWidget>();
}

void ToolSelectWidget::setupLayout() {
  auto *layout = new QVBoxLayout{box};
  layout->setSpacing(0);
  layout->setContentsMargins(glob_margin, glob_margin, glob_margin, glob_margin);
  layout->addStretch();
  for (ToolWidget *tool : tools) {
    layout->addWidget(tool);
  }
  layout->addStretch();
}

void ToolSelectWidget::connectSignals() {
  CONNECT(ctx, cellModified,    this, cellModified);
  CONNECT(ctx, overlayModified, this, overlayModified);
  CONNECT(ctx, shouldShowNorm,  this, shouldShowNorm);
  CONNECT(ctx, changingAction,  this, changingAction);
  CONNECT(ctx, cellRequested,   this, cellRequested);
  CONNECT(ctx, lockRequested,   this, lockRequested);
  CONNECT(ctx, unlockRequested, this, unlockRequested);
  CONNECT(ctx, lockRequested,   this, lockTool);
  CONNECT(ctx, unlockRequested, this, unlockTool);
  for (ToolWidget *tool : tools) {
    CONNECT(tool, shouldChangeTool, this, setTool);
  }
  tools[0]->click();
}

#include "tool select widget.moc"
