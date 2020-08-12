//
//  tools select widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 10/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "tool select widget.hpp"

#include "connect.hpp"
#include "config keys.hpp"
#include <QtGui/qbitmap.h>
#include "tool widgets.hpp"
#include <QtGui/qpainter.h>
#include "config colors.hpp"
#include "config geometry.hpp"
#include "widget painting.hpp"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qscrollbar.h>
#include "radio button widget.hpp"
#include "tool param bar widget.hpp"

class ToolWidget final : public RadioButtonWidget {
  Q_OBJECT
  
public:
  template <typename ParamWidget>
  ToolWidget(QWidget *parent, std::unique_ptr<Tool> tool, ParamWidget *params)
    : RadioButtonWidget{parent},
      tool{std::move(tool)},
      params{params} {
    loadIcons(ParamWidget::icon_name);
    setToolTip(ParamWidget::tooltip);
    setFixedSize(tool_button_size);
    CONNECT(this, toggled, this, changeTool);
  }

Q_SIGNALS:
  void shouldChangeTool(ToolWidget *, Tool *);

private Q_SLOTS:
  void changeTool(const bool checked) {
    if (checked) {
      Q_EMIT shouldChangeTool(this, tool.get());
      params->show();
    } else {
      params->hide();
    }
  }

private:
  std::unique_ptr<Tool> tool;
  ToolParamWidget *params;
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

ToolSelectWidget::ToolSelectWidget(QWidget *parent, ToolParamBarWidget *bar)
  : QScrollArea{parent}, bar{bar}, box{new QWidget{this}} {
  setFixedWidth(tool_select_width);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setStyleSheet("background-color:" + tool_select_background.name());
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

void ToolSelectWidget::setCelImage(CelImage *cel) {
  ctx.cel = cel;
  if (mouseIn) currTool.mouseLeave();
  currTool.setCel(cel);
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

void ToolSelectWidget::resizeCanvas(const QSize size) {
  ctx.size = size;
}

void ToolSelectWidget::setTool(ToolWidget *widget, Tool *tool) {
  ensureVisible(widget);
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
  ensureWidgetVisible(tool, glob_margin, glob_margin);
  update();
}

namespace {

template <typename ParamWidget, typename Tool>
auto init(ParamWidget *params, Tool *tool, int)
  -> std::void_t<decltype(params->createWidgets())> {
  params->createWidgets();
  params->setupLayout();
  params->connectSignals(tool);
}

template <typename ParamWidget, typename Tool>
void init(ParamWidget *, Tool *, long) {}

}

template <typename ParamWidget>
void ToolSelectWidget::pushToolWidget(QHBoxLayout *layout) {
  auto tool = std::make_unique<typename ParamWidget::impl>();
  tool->setCtx(&ctx);
  auto *params = new ParamWidget{bar};
  layout->addWidget(params);
  init(params, tool.get(), 0);
  params->hide();
  tools.push_back(new ToolWidget{box, std::move(tool), params});
}

void ToolSelectWidget::createTools() {
  auto *layout = new QHBoxLayout{bar};
  layout->setSpacing(0);
  layout->setContentsMargins(glob_margin, 0, glob_margin, 0);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->addStretch();

  pushToolWidget<BrushToolWidget>(layout);
  pushToolWidget<FloodFillToolWidget>(layout);
  pushToolWidget<RectangleSelectToolWidget>(layout);
  pushToolWidget<PolygonSelectToolWidget>(layout);
  pushToolWidget<WandSelectToolWidget>(layout);
  pushToolWidget<LineToolWidget>(layout);
  pushToolWidget<StrokedCircleToolWidget>(layout);
  pushToolWidget<FilledCircleToolWidget>(layout);
  pushToolWidget<StrokedRectangleToolWidget>(layout);
  pushToolWidget<FilledRectangleToolWidget>(layout);
  pushToolWidget<LinearGradientToolWidget>(layout);
  pushToolWidget<TranslateToolWidget>(layout);
  pushToolWidget<FlipToolWidget>(layout);
  pushToolWidget<RotateToolWidget>(layout);
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
  CONNECT(ctx, celImageModified,     this, celImageModified);
  CONNECT(ctx, overlayModified,      this, overlayModified);
  CONNECT(ctx, shouldShowNorm,       this, shouldShowNorm);
  CONNECT(ctx, changingAction,       this, changingAction);
  CONNECT(ctx, shouldGrowCelImage,   this, shouldGrowCelImage);
  CONNECT(ctx, shouldShrinkCelImage, this, shouldShrinkCelImage);
  CONNECT(ctx, shouldLock,           this, shouldLock);
  CONNECT(ctx, shouldUnlock,         this, shouldUnlock);
  CONNECT(ctx, shouldLock,           this, lockTool);
  CONNECT(ctx, shouldUnlock,         this, unlockTool);
  for (ToolWidget *tool : tools) {
    CONNECT(tool, shouldChangeTool, this, setTool);
  }
  tools[0]->click();
  bar->adjustSize();
}

#include "tool select widget.moc"
