//
//  tool.hpp
//  Animera
//
//  Created by Indiana Kernick on 18/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_tool_hpp
#define animera_tool_hpp

#include "image.hpp"
#include "status msg.hpp"
#include "palette span.hpp"
#include <QtCore/qobject.h>

class Cell;

enum class ButtonType {
  none,
  primary,
  secondary,
  erase
};

enum class ToolChanges {
  none,
  cell,
  overlay,
  cell_overlay
};

struct ToolColors {
  QRgb primary;
  QRgb secondary;
  QRgb erase;
};

struct ToolLeaveEvent {
  QPoint lastPos;
  ButtonType button;
};

struct ToolMouseDownEvent {
  QPoint pos;
  ButtonType button;
};

struct ToolMouseMoveEvent {
  QPoint pos;
  QPoint lastPos;
  ButtonType button;
};

struct ToolMouseUpEvent {
  QPoint pos;
  ButtonType button;
};

struct ToolKeyEvent {
  Qt::Key key;
};

class ToolCtx final : public QObject {
  Q_OBJECT

Q_SIGNALS:
  void cellModified(QRect) const;
  void overlayModified(QRect) const;
  void shouldShowNorm(std::string_view) const;
  void changingAction() const;
  void growRequested(QRect) const;
  void shrinkRequested(QRect) const;
  void lockRequested() const;
  void unlockRequested() const;

public:
  Cell *cell = nullptr;
  QImage *overlay = nullptr;
  PaletteCSpan palette;
  Format format;
  QSize size;
  ToolColors colors;
  
  void changeCell(QRect) const;
  void changeCell(QPoint) const;
  void changeOverlay(QRect) const;
  void changeOverlay(QPoint) const;
  
  void growCell(QRect) const;
  void shrinkCell(QRect) const;
  [[deprecated]] void shrinkCell() const;
  
  QRgb selectColor(ButtonType) const;
  
  void showStatus(const StatusMsg &) const;
  void clearStatus() const;
  
  void finishChange() const;
  
  void lock() const;
  void unlock() const;
};

class Tool {
public:
  virtual ~Tool() = default;

  virtual void attachCell() {}
  virtual void detachCell() {}
  virtual void mouseLeave(const ToolLeaveEvent &) {}
  virtual void mouseDown(const ToolMouseDownEvent &) {}
  virtual void mouseMove(const ToolMouseMoveEvent &) {}
  virtual void mouseUp(const ToolMouseUpEvent &) {}
  virtual void keyPress(const ToolKeyEvent &) {}
  
  void setCtx(const ToolCtx *);

protected:
  const ToolCtx *ctx = nullptr;
};

#endif
