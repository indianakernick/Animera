//
//  animation.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef animation_hpp
#define animation_hpp

#include "cell.hpp"
#include <QtCore/qobject.h>

// @TODO Most of this should be in the TimelineWidget
// The timeline should be in charge of the cells that make up the project

class Animation : public QObject {
public:
  void setPalette(Palette *);

  bool hasLayer(LayerIdx) const;
  bool hasFrame(CellPos) const;
  bool hasFrame(LayerIdx, FrameIdx) const;
  LayerIdx layerCount() const;
  FrameIdx frameCount(LayerIdx) const;

  Cell *getCell(CellPos) const;
  Cell *getCell(LayerIdx, FrameIdx) const;
  Frame getFrame(FrameIdx) const;
  const Palette &getPallete() const;

  Layers copyRect(CellRect) const;
  void removeRect(CellRect);
  void clearRect(CellRect);
  void pasteRect(CellRect, const Layers &);

  void appendLayer();

private:
  Layers layers;
  QSize size;
  Palette *palette = nullptr;
  Format format;
  
  const Cell *getLastCell(LayerIdx) const;
  void removeTrailingNull(LayerIdx);
  void updateLayer(LayerIdx);
  bool validRect(CellRect) const;
};

#endif
