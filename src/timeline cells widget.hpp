//
//  timeline cells widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef timeline_cells_widget_hpp
#define timeline_cells_widget_hpp

#include "cell.hpp"
#include <QtWidgets/qwidget.h>
#include "scroll bar widget.hpp"

class TimelineWidget;
class QVBoxLayout;

class LayerCellsWidget final : public QWidget {
public:
  // @TODO a sparse data structure might be better
  struct LinkedSpan {
    // Does this need to be a std::unique_ptr?
    CellPtr cell;
    FrameIdx len = 1;
  };

  LayerCellsWidget(QWidget *, TimelineWidget *);

  Cell *appendCell(FrameIdx = 1);
  void appendNull(FrameIdx = 1);
  void appendFrame();
  Cell *getCell(FrameIdx);
  
  void serialize(QIODevice *) const;
  void deserialize(QIODevice *);

private:
  TimelineWidget &timeline;
  std::vector<LinkedSpan> frames;
  QPixmap cellPix;
  QPixmap beginLinkPix;
  QPixmap endLinkPix;
  
  void loadIcons();
  void addSize(FrameIdx);
  const Cell *getLastCell() const;
  
  void paintBorder(QPainter &, int);
  void paintEvent(QPaintEvent *) override;
};

class CellsWidget final : public QWidget {
  Q_OBJECT
  
public:
  CellsWidget(QWidget *, TimelineWidget *);

Q_SIGNALS:
  void resized();

public Q_SLOTS:
  void changeWidth(int);

public:
  LayerCellsWidget *appendLayer();
  LayerCellsWidget *getLayer(LayerIdx);
  void appendFrame();
  LayerIdx layerCount() const;
  
  void serialize(QIODevice *) const;
  void deserialize(QIODevice *);
  
private:
  TimelineWidget *timeline;
  QVBoxLayout *layout;
  std::vector<LayerCellsWidget *> layers;
  
  void resizeEvent(QResizeEvent *) override;
};

class CellScrollWidget final : public ScrollAreaWidget {
  Q_OBJECT
  
public:
  explicit CellScrollWidget(QWidget *);

Q_SIGNALS:
  void rightMarginChanged(int);
  void bottomMarginChanged(int);

public Q_SLOTS:
  void contentResized();

private:
  void resizeEvent(QResizeEvent *) override;
};

#endif
