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
  void cellModified();
  void overlayModified();
  void shouldShowPerm(std::string_view);
  void changingAction();
  void cellRequested();

public Q_SLOTS:
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
  void changeTool(ToolWidget *, Tool *);
  
private:
  QWidget *box;
  std::vector<ToolWidget *> tools;
  CurrentTool currTool;
  // currWidget only exists to accomodate future changes (tool settings)
  ToolWidget *currWidget = nullptr;
  ToolCtx ctx;
  
  template <typename WidgetClass>
  void pushToolWidget();
  void createTools();
  void setupLayout();
  void connectSignals();
};

#endif
