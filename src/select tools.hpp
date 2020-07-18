//
//  select tools.hpp
//  Animera
//
//  Created by Indiana Kernick on 14/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_select_tools_hpp
#define animera_select_tools_hpp

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
  QRect overlayRect(QPoint);
  void showOverlay(QPoint);
  void clearOverlay(QPoint);
  void clearOverlay(SelectMode, QPoint);
  void toggleMode();
  
  QImage selection;
  QImage overlay;
  QPoint offset;
  QRect bounds;
  QRect lastBounds;
  SelectMode mode = SelectMode::copy;
};

class RectangleSelectTool final : public SelectTool<RectangleSelectTool> {
public:
  ~RectangleSelectTool();

  void attachCelImage() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseDownEvent &) override;
  void mouseMove(const ToolMouseMoveEvent &) override;
  void mouseUp(const ToolMouseUpEvent &) override;
  
private:
  QPoint startPos = no_point;
};

class PolygonSelectTool final : public SelectTool<PolygonSelectTool> {
public:
  ~PolygonSelectTool();

  void attachCelImage() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseDownEvent &) override;
  void mouseMove(const ToolMouseMoveEvent &) override;
  void mouseUp(const ToolMouseUpEvent &) override;
  
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
  ~WandSelectTool();
  
  void attachCelImage() override;
  void detachCelImage() override;
  void mouseLeave(const ToolLeaveEvent &) override;
  void mouseDown(const ToolMouseDownEvent &) override;
  void mouseMove(const ToolMouseMoveEvent &) override;

private:
  QImage mask;
  QTimer animTimer;
  int animFrame;
  
  QRect celRect() const;
  void toggleMode(const ToolMouseDownEvent &);
  void addToSelection(const ToolMouseDownEvent &);
  QRgb getOverlayColor() const;
  void paintOverlay() const;
  void animate();
};

#endif
