//
//  select tools.hpp
//  Animera
//
//  Created by Indi Kernick on 14/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef select_tools_hpp
#define select_tools_hpp

#include "tool.hpp"
#include <QtCore/qtimer.h>

template <typename Derived>
class SelectTool : public Tool {
public:
  ~SelectTool();
  
protected:
  bool resizeImages();
  void copy(QPoint);
  void copyWithMask(QPoint, const QImage &);
  void paste(QPoint, ButtonType);
  void pasteWithMask(QPoint, ButtonType, const QImage &);
  void showOverlay(QPoint);
  void toggleMode();
  
  QImage selection;
  QImage overlay;
  QPoint offset;
  QRect bounds;
  SelectMode mode = SelectMode::copy;
};

class RectangleSelectTool final : public SelectTool<RectangleSelectTool> {
public:
  void attachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;
  void mouseUp(const ToolMouseEvent &) override;
  
private:
  QPoint startPos = no_point;
};

// TODO: is polygon select really all that useful?
class PolygonSelectTool final : public SelectTool<PolygonSelectTool> {
public:
  void attachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;
  void mouseUp(const ToolMouseEvent &) override;
  
private:
  QImage mask;
  std::vector<QPoint> polygon;
  
  void initPoly(QPoint);
  void pushPoly(QPoint);
};

// TODO: What if you could remove from the selection by pressing undo?
class WandSelectTool final : public SelectTool<WandSelectTool> {
public:
  WandSelectTool();
  
  void attachCell() override;
  void detachCell() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseEvent &) override;
  void mouseMove(const ToolMouseEvent &) override;

private:
  QImage mask;
  QTimer animTimer;
  int animFrame;
  
  void toggleMode(const ToolMouseEvent &);
  void addToSelection(const ToolMouseEvent &);
  QRgb getOverlayColor() const;
  void paintOverlay() const;
  void animate();
};

#endif
