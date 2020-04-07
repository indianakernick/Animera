//
//  timeline cells widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 24/6/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef timeline_cells_widget_hpp
#define timeline_cells_widget_hpp

#include <span>
#include "cell.hpp"
#include <QtCore/qtimer.h>
#include "scroll bar widget.hpp"

class CellsWidget final : public QWidget {
  Q_OBJECT
  
public:
  explicit CellsWidget(QWidget *);

Q_SIGNALS:
  void resized();
  void shouldEnsureVisible(QPoint);
  void beginSelection();
  void continueSelection();
  void endSelection();
  void clearSelection();
  void currPosChanged(CellPos);
  
public Q_SLOTS:
  void setSelection(CellRect);
  void setCurrPos(CellPos);
  void setLayer(LayerIdx, std::span<const CellSpan>);
  void setFrameCount(FrameIdx);
  void setLayerCount(LayerIdx);

private:
  QPixmap cellPix;
  QPixmap beginLinkPix;
  QPixmap endLinkPix;
  
  QImage currPosImg;
  QImage selectionImg;
  QImage bordersImg;
  QImage layersImg;
  
  QRect selectionRect;
  
  void updateSelectionImg();

  void paintEvent(QPaintEvent *) override;
  void focusOutEvent(QFocusEvent *) override;
  
  CellPos getPos(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
};

class CellScrollWidget final : public ScrollAreaWidget {
  Q_OBJECT
  
public:
  explicit CellScrollWidget(QWidget *);

  CellsWidget *getChild();

Q_SIGNALS:
  void rightMarginChanged(int);
  void bottomMarginChanged(int);

public Q_SLOTS:
  void changeMargins();
  void ensureVisible(QPoint);

private:
  QWidget *rect = nullptr;

  void resizeEvent(QResizeEvent *) override;
};

#endif
