//
//  tool select widget.hpp
//  Animera
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef tool_select_widget_hpp
#define tool_select_widget_hpp

#include "status msg.hpp"
#include "current tool.hpp"
#include <QtWidgets/qscrollarea.h>

class ToolWidget;

class ToolSelectWidget final : public QScrollArea {
  Q_OBJECT

  friend class ToolWidget;

public:
  explicit ToolSelectWidget(QWidget *);

Q_SIGNALS:
  void cellModified(QRect);
  void overlayModified(QRect);
  void shouldShowNorm(std::string_view);
  void changingAction();
  void cellRequested(QRect);
  void lockRequested();
  void unlockRequested();

public Q_SLOTS:
  void mouseEnter(QPoint);
  void mouseLeave();
  void mouseDown(QPoint, ButtonType);
  void mouseMove(QPoint);
  void mouseUp(QPoint, ButtonType);
  void keyPress(Qt::Key);
  
  void setOverlay(QImage *);
  void setCell(Cell *);
  void setColors(ToolColors);
  void setPalette(PaletteCSpan);
  void initCanvas(Format, QSize);
  
private Q_SLOTS:
  void setTool(ToolWidget *, Tool *);
  void lockTool();
  void unlockTool();
  
private:
  QWidget *box = nullptr;
  std::vector<ToolWidget *> tools;
  CurrentTool currTool;
  ToolWidget *currWidget = nullptr;
  ToolCtx ctx;
  bool mouseIn = false;
  
  void ensureVisible(ToolWidget *);
  template <typename WidgetClass>
  void pushToolWidget();
  void createTools();
  void setupLayout();
  void connectSignals();
};

#endif
