//
//  timeline.hpp
//  Animera
//
//  Created by Indi Kernick on 6/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef timeline_hpp
#define timeline_hpp

#include <span>
#include "png.hpp"
#include "palette.hpp"
#include "cell span.hpp"
#include "export options.hpp"

// @TODO can we make the interface of Timeline smaller?

/*

struct LayerData {
  LayerCells spans;
  std::string name;
  bool visible;
};

class Layer {
public:
  // signals and slots
 
private:
  LayerData *data;
};

*/

class Timeline final : public QObject {
  Q_OBJECT

public:
  Timeline();

private:
  struct Layer;

  static void writeLHDR(QIODevice &, const Layer &);
  static void writeCHDR(QIODevice &, const CellSpan &);
  static void writeCDAT(QIODevice &, const QImage &, Format);

public:

  void initDefault();
  void serializeHead(QIODevice &) const;
  void serializeBody(QIODevice &) const;
  void serializeTail(QIODevice &) const;
  void deserialize(QIODevice *);

private:
  CellRect selectCells(const ExportOptions &) const;
  QImage convertImage(ExportFormat, PaletteCSpan, QImage) const;
  void exportFile(const ExportOptions &, PaletteCSpan, QImage, CellPos) const;
  void exportCompRect(const ExportOptions &, PaletteCSpan, CellRect) const;
  void exportRect(const ExportOptions &, PaletteCSpan, CellRect) const;

public:
  void exportTimeline(const ExportOptions &, PaletteCSpan) const;

public Q_SLOTS:
  void initCanvas(Format, QSize);

  void nextFrame();
  void prevFrame();
  void layerBelow();
  void layerAbove();
  
  void beginSelection();
  void continueSelection();
  void endSelection();
  void clearSelection();
  
  void insertLayer();
  void removeLayer();
  void moveLayerUp();
  void moveLayerDown();
  
  void insertFrame();
  void insertNullFrame();
  void removeFrame();
  
  void clearCell();
  void extendCell();
  void splitCell();
  void requestCell();
  
  void setCurrPos(CellPos);
  void setVisibility(LayerIdx, bool);
  void setName(LayerIdx, std::string_view);
  
  void clearSelected();
  void copySelected();
  void pasteSelected();

Q_SIGNALS:
  void currPosChanged(CellPos);
  void currCellChanged(Cell *);
  void selectionChanged(CellRect);
  
  void visibilityChanged(LayerIdx, bool);
  void nameChanged(LayerIdx, std::string_view);
  
  void frameChanged(const Frame &);
  void layerChanged(LayerIdx, std::span<const CellSpan>);
  
  void frameCountChanged(FrameIdx);
  void layerCountChanged(LayerIdx);
  
  void modified();
  
private:
  struct Layer {
    LayerCells spans;
    std::string name;
    bool visible = true;
  };

  std::vector<Layer> layers;
  CellPos currPos;
  CellRect selection;
  FrameIdx frameCount;
  QSize canvasSize;
  Format canvasFormat;
  std::vector<LayerCells> clipboard;
  
  CellPtr makeCell() const;
  Cell *getCell(CellPos);
  Frame getFrame(FrameIdx) const;
  LayerIdx layerCount() const;
  
  void changePos();
  void changeFrame();
  void changeSpan(LayerIdx);
  void changeLayers(LayerIdx, LayerIdx);
  void changeFrameCount();
  void changeLayerCount();
};

#endif
