//
//  sprite export.cpp
//  Animera
//
//  Created by Indi Kernick on 5/10/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "sprite export.hpp"

#include <QtCore/qdir.h>
#include "composite.hpp"
#include "export png.hpp"
#include "export pattern.hpp"

Exporter::Exporter(
  const ExportOptions &options,
  const PaletteCSpan palette,
  const Format format,
  const QSize size
) : options{options},
    palette{palette},
    format{format},
    image{size, qimageFormat(format)},
    rect{empty_rect} {}

void Exporter::setRect(
  const LayerIdx layerCount,
  const FrameIdx frameCount,
  const CellPos currPos
) {
  switch (options.layerSelect) {
    case LayerSelect::all_composited:
    case LayerSelect::all:
      rect.minL = LayerIdx{0};
      rect.maxL = layerCount - LayerIdx{1};
      break;
    case LayerSelect::current:
      rect.minL = rect.maxL = currPos.l;
      break;
    default: Q_UNREACHABLE();
  }
  switch (options.frameSelect) {
    case FrameSelect::all:
      rect.minF = FrameIdx{0};
      rect.maxF = frameCount - FrameIdx{1};
      break;
    case FrameSelect::current:
      rect.minF = rect.maxF = currPos.f;
      break;
    default: Q_UNREACHABLE();
  }
}

Error Exporter::exportSprite(const std::vector<Layer> &layers) {
  assert(
    rect.minL != empty_rect.minL ||
    rect.minF != empty_rect.minF ||
    rect.maxL != empty_rect.maxL ||
    rect.maxF != empty_rect.maxF
  );
  if (composited(options.layerSelect)) {
    return exportFrames(layers);
  } else {
    return exportCells(layers);
  }
}

void Exporter::setImageFrom(const Cell &cell) {
  clearImage(image);
  blitImage(image, cell.img, cell.pos);
}

void Exporter::setImageFrom(const Frame &frame) {
  if (format == Format::gray) {
    compositeFrame<gfx::YA>(image, palette, frame, format);
  } else {
    compositeFrame<gfx::ARGB>(image, palette, frame, format);
  }
}

namespace {

template <typename Idx>
Idx apply(const Line<Idx> line, const Idx value) {
  return value * line.stride + line.offset;
}

}

QString Exporter::getPath(CellPos pos) {
  QString path = options.directory;
  if (path.back() != QDir::separator()) {
    path.push_back(QDir::separator());
  }
  pos.l = apply(options.layerLine, pos.l);
  pos.f = apply(options.frameLine, pos.f);
  path += evalExportPattern(options.name, pos.l, pos.f);
  path += ".png";
  return path;
}

Error Exporter::exportImage(const CellPos pos) {
  return exportPng(getPath(pos), palette, image, format, options.format);
}

Error Exporter::exportCells(const std::vector<Layer> &layers) {
  for (LayerIdx l = rect.minL; l <= rect.maxL; ++l) {
    const Layer &layer = layers[+l];
    // TODO: does the user want to skip invisible layers?
    if (!layer.visible) continue;
    LayerCells::ConstIterator iter = layer.spans.find(rect.minF);
    for (FrameIdx f = rect.minF; f <= rect.maxF; ++f) {
      if (const Cell *cell = *iter; *cell) {
        setImageFrom(*cell);
        if (Error err = exportImage({l, f}); err) {
          return err;
        }
      }
      ++iter;
    }
  }
  return {};
}

Error Exporter::exportFrames(const std::vector<Layer> &layers) {
  const LayerIdx rectLayers = rect.maxL - rect.minL + LayerIdx{1};
  Frame frame;
  frame.reserve(+rectLayers);
  std::vector<LayerCells::ConstIterator> iterators;
  iterators.reserve(+rectLayers);
  for (LayerIdx l = rect.minL; l <= rect.maxL; ++l) {
    iterators.push_back(layers[+l].spans.find(rect.minF));
  }
  for (FrameIdx f = rect.minF; f <= rect.maxF; ++f) {
    frame.clear();
    for (LayerIdx l = {}; l != rectLayers; ++l) {
      if (!layers[+(l + rect.minL)].visible) continue;
      if (const Cell *cell = *iterators[+l]; *cell) {
        frame.push_back(*iterators[+l]);
      }
      ++iterators[+l];
    }
    setImageFrom(frame);
    if (Error err = exportImage({rect.minL, f}); err) {
      return err;
    }
  }
  return {};
}
