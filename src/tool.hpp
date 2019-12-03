//
//  tool.hpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef tool_hpp
#define tool_hpp

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

struct ToolMouseEvent {
  QPoint pos;
  QPoint lastPos;
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
  void shrinkRequested() const;
  void lockRequested() const;
  void unlockRequested() const;

public:
  Cell *cell = nullptr;
  QImage *overlay = nullptr;
  PaletteCSpan palette;
  Format format;
  QSize size;
  ToolColors colors;
  
  [[deprecated]] void emitChanges(ToolChanges) const;
  [[deprecated]] void emitChanges(bool) const;
  
  void changeCell(QRect) const;
  void changeCell(QPoint) const;
  void changeOverlay(QRect) const;
  void changeOverlay(QPoint) const;
  
  void growCell(QRect) const;
  void shrinkCell() const;
  
  QRgb selectColor(ButtonType) const;
  
  [[deprecated]] void showStatus(const StatusMsg &) const;
  StatusMsg showStatus() const;
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
  virtual void mouseDown(const ToolMouseEvent &) {}
  virtual void mouseMove(const ToolMouseEvent &) {}
  virtual void mouseUp(const ToolMouseEvent &) {}
  virtual void keyPress(const ToolKeyEvent &) {}
  
  void setCtx(const ToolCtx *);

protected:
  const ToolCtx *ctx = nullptr;
};

#endif
