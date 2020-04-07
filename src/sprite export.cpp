//
//  sprite export.cpp
//  Animera
//
//  Created by Indiana Kernick on 5/10/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "sprite export.hpp"

#include "file io.hpp"
#include <QtCore/qdir.h>
#include "composite.hpp"
#include "export png.hpp"
#include "scope time.hpp"
#include "surface factory.hpp"
#include "graphics convert.hpp"
#include <Graphics/transform.hpp>

Exporter::Exporter(
  const ExportOptions &options,
  const PaletteCSpan palette,
  const Format format,
  const QSize size
) : options{options},
    palette{palette},
    format{format},
    size{size} {}

Error Exporter::exportSprite(const std::vector<Layer> &layers) {
  SCOPE_TIME("Exporter::exportSprite");
  
  initImages();
  if (options.composite) {
    return exportFrames(layers);
  } else {
    return exportCells(layers);
  }
}

void Exporter::initImages() {
  Format imageFormat = format;
  if (options.composite && format != Format::gray) {
    imageFormat = Format::rgba;
  }
  image = {size, qimageFormat(imageFormat)};
  xformed = {getXformedSize(), qimageFormat(imageFormat)};
}

QSize Exporter::getXformedSize() const {
  if (options.scaleX == 1 && options.scaleY == 1 && options.angle == 0) {
    return {0, 0};
  }
  QSize xformedSize;
  xformedSize.setWidth(image.width() * std::abs(options.scaleX));
  xformedSize.setHeight(image.height() * std::abs(options.scaleY));
  xformedSize = convert(gfx::rotateSize(convert(xformedSize), options.angle));
  return xformedSize;
}

void Exporter::setImageFrom(const Cell &cell) {
  SCOPE_TIME("Exporter::setImageFrom (cell)");
  
  clearImage(image);
  blitImage(image, cell.img, cell.pos);
}

void Exporter::setImageFrom(const Frame &frame) {
  SCOPE_TIME("Exporter::setImageFrom (frame)");

  if (format == Format::gray) {
    compositeFrame<gfx::YA>(image, palette, frame, format, image.rect());
  } else {
    compositeFrame<gfx::ARGB>(image, palette, frame, format, image.rect());
  }
}

void Exporter::applyTransform() {
  SCOPE_TIME("Exporter::applyTransform");

  visitSurface(xformed, [this](const auto dst) {
    const auto src = makeCSurface<typename decltype(dst)::Pixel>(image);
    gfx::spatialTransform(dst, src, [this, &dst](const gfx::Point dstPos) noexcept {
      gfx::Point srcPos = gfx::rotate(options.angle, dst.size(), dstPos);
      srcPos = options.scaleX < 0 ? gfx::flipHori(dst.size(), srcPos) : srcPos;
      srcPos = options.scaleY < 0 ? gfx::flipVert(dst.size(), srcPos) : srcPos;
      return gfx::scale({std::abs(options.scaleX), std::abs(options.scaleY)}, srcPos);
    });
  });
}

Error Exporter::exportImage(const ExportState state) {
  SCOPE_TIME("Exporter::exportImage");

  FileWriter writer;
  TRY(writer.open(getExportPath(options, state)));
  if (xformed.isNull()) {
    TRY(exportCellPng(writer.dev(), palette, image, format, options.format));
  } else {
    applyTransform();
    TRY(exportCellPng(writer.dev(), palette, xformed, format, options.format));
  }
  return writer.flush();
}

bool Exporter::shouldInclude(const Layer &layer) const {
  switch (options.visibility) {
    case ExportVis::visible: return layer.visible;
    case ExportVis::hidden: return !layer.visible;
    case ExportVis::all: return true;
  }
}

Error Exporter::exportCells(const std::vector<Layer> &layers) {
  SCOPE_TIME("Exporter::exportCells");
  
  for (LayerIdx l = options.selection.minL; l <= options.selection.maxL; ++l) {
    const Layer &layer = layers[+l];
    if (!shouldInclude(layer)) continue;
    LayerCells::ConstIterator iter = layer.spans.find(options.selection.minF);
    for (FrameIdx f = options.selection.minF; f <= options.selection.maxF; ++f) {
      if (const Cell *cell = *iter; *cell) {
        setImageFrom(*cell);
        TRY(exportImage({l, f}));
      }
      ++iter;
    }
  }
  return {};
}

Error Exporter::exportFrames(const std::vector<Layer> &layers) {
  SCOPE_TIME("Exporter::exportFrames");

  const LayerIdx rectLayers = options.selection.maxL - options.selection.minL + LayerIdx{1};
  Frame frame;
  frame.reserve(+rectLayers);
  std::vector<LayerCells::ConstIterator> iterators;
  iterators.reserve(+rectLayers);
  for (LayerIdx l = options.selection.minL; l <= options.selection.maxL; ++l) {
    iterators.push_back(layers[+l].spans.find(options.selection.minF));
  }
  for (FrameIdx f = options.selection.minF; f <= options.selection.maxF; ++f) {
    frame.clear();
    for (LayerIdx l = {}; l != rectLayers; ++l) {
      if (!shouldInclude(layers[+(l + options.selection.minL)])) continue;
      if (const Cell *cell = *iterators[+l]; *cell) {
        frame.push_back(*iterators[+l]);
      }
      ++iterators[+l];
    }
    setImageFrom(frame);
    TRY(exportImage({options.selection.minL, f}));
  }
  return {};
}
