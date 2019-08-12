//
//  timeline.cpp
//  Animera
//
//  Created by Indi Kernick on 6/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "timeline.hpp"

#include "serial.hpp"
#include "formats.hpp"
#include "transform.hpp"
#include "cell span.hpp"
#include "composite.hpp"
#include <QtCore/qdir.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qendian.h>
#include "export pattern.hpp"
#include "surface factory.hpp"

namespace {

constexpr CellRect empty_rect = {LayerIdx{0}, FrameIdx{0}, LayerIdx{-1}, FrameIdx{-1}};

}

Timeline::Timeline()
  : currPos{LayerIdx{0}, FrameIdx{0}}, frameCount{0} {}

void Timeline::initDefault() {
  frameCount = FrameIdx{1};
  changeFrameCount();
  Layer layer;
  layer.spans.pushCell(makeCell());
  layer.name = "Layer 0";
  layers.push_back(std::move(layer));
  selection = empty_rect;
  changeLayerCount();
  changeFrame();
  changePos();
  Q_EMIT selectionChanged(selection);
  changeLayers(LayerIdx{0}, LayerIdx{1});
}

namespace {

void deserializeImage(QIODevice *dev, QImage &image) {
  assert(dev);
  image.load(dev, "png");
}

constexpr int byteDepth(const Format format) {
  switch (format) {
    case Format::rgba:
      return 4;
    case Format::index:
      return 1;
    case Format::gray:
      return 2;
  }
}

uint8_t formatByte(const Format format) {
  return byteDepth(format);
}

}

void Timeline::writeLHDR(QIODevice &dev, const Layer &layer) {
  ChunkWriter writer{dev};
  const uint32_t nameLen = static_cast<uint32_t>(layer.name.size());
  writer.begin(2 * 4 + 1 + nameLen, "LHDR");
  writer.writeInt(static_cast<uint32_t>(layer.spans.size()));
  writer.writeInt(nameLen);
  writer.writeByte(layer.visible);
  writer.writeString(layer.name.data(), nameLen);
  writer.end();
}

void Timeline::writeCHDR(QIODevice &dev, const CellSpan &span) {
  ChunkWriter writer{dev};
  writer.begin(4 * 4, "CHDR");
  writer.writeInt(static_cast<uint32_t>(span.len));
  if (span.cell) {
    writer.writeInt(0); // x
    writer.writeInt(0); // y
    writer.writeInt(span.cell->image.width());
    writer.writeInt(span.cell->image.height());
  } else {
    writer.writeInt(0); // x
    writer.writeInt(0); // y
    writer.writeInt(0); // width
    writer.writeInt(0); // height
  }
  writer.end();
}

namespace {

void initStream(z_streamp stream) {
  stream->zalloc = nullptr;
  stream->zfree = nullptr;
  [[maybe_unused]] const int err = deflateInit(stream, Z_DEFAULT_COMPRESSION);
  assert(err == Z_OK);
  // The errors returned by deflateInit should never happen
}

void byteOrderCopy(uchar *dst, const uchar *src, const size_t size, const Format format) {
  switch (format) {
    case Format::rgba: {
      auto *srcPx = reinterpret_cast<const FormatARGB::Pixel *>(src);
      uchar *dstEnd = dst + size;
      while (dst != dstEnd) {
        const Color color = FormatARGB::toColor(*srcPx);
        dst[0] = color.r;
        dst[1] = color.g;
        dst[2] = color.b;
        dst[3] = color.a;
        ++srcPx;
        dst += 4;
      }
      break;
    }
    case Format::index:
      static_assert(sizeof(FormatPalette::Pixel) == 1);
      std::memcpy(dst, src, size);
      break;
    case Format::gray: {
      auto *srcPx = reinterpret_cast<const FormatGray::Pixel *>(src);
      uchar *dstEnd = dst + size;
      while (dst != dstEnd) {
        const Color color = FormatGray::toColor(*srcPx);
        dst[0] = color.r;
        dst[1] = color.a;
        ++srcPx;
        dst += 4;
      }
      break;
    }
  }
}

}

void Timeline::writeCDAT(QIODevice &dev, const QImage &image, const Format format) {
  assert(!image.isNull());
  const uint32_t outBuffSize = 1 << 16;
  std::vector<Bytef> outBuff(outBuffSize);
  const uint32_t inBuffSize = image.width() * byteDepth(format);
  std::vector<Bytef> inBuff(inBuffSize);
  
  // @TODO avoid overflowing uint32_t
  // might need to reduce maximum image size
  // or split the image data into multiple chunks
  
  z_stream stream;
  initStream(&stream);
  stream.avail_in = 0;
  stream.next_out = outBuff.data();
  stream.avail_out = outBuffSize;
  
  int rowIdx = -1;
  int ret;
  
  ChunkWriter writer{dev};
  writer.begin("CDAT");
  
  do {
    if (stream.avail_in == 0) {
      ++rowIdx;
      if (rowIdx < image.height()) {
        byteOrderCopy(inBuff.data(), image.scanLine(rowIdx), inBuffSize, format);
        stream.next_in = inBuff.data();
        stream.avail_in = inBuffSize;
      }
    }
    if (stream.avail_out == 0) {
      writer.writeString(outBuff.data(), outBuffSize);
      stream.next_out = outBuff.data();
      stream.avail_out = outBuffSize;
    }
    ret = deflate(&stream, stream.avail_in ? Z_NO_FLUSH : Z_FINISH);
  } while (ret == Z_OK);
  assert(ret == Z_STREAM_END);
  
  if (stream.avail_out < outBuffSize) {
    writer.writeString(outBuff.data(), outBuffSize - stream.avail_out);
  }
  
  writer.end();
  
  ret = deflateEnd(&stream);
  assert(ret == Z_OK);
}

void Timeline::serializeHead(QIODevice &dev) const {
  ChunkWriter writer{dev};
  writer.begin(5 * 4 + 1, "AHDR");
  writer.writeInt(canvasSize.width());
  writer.writeInt(canvasSize.height());
  writer.writeInt(static_cast<uint32_t>(layerCount()));
  writer.writeInt(static_cast<uint32_t>(frameCount));
  writer.writeInt(100); // delay
  writer.writeByte(formatByte(canvasFormat));
  writer.end();
}

void Timeline::serializeBody(QIODevice &dev) const {
  for (const Layer &layer : layers) {
    writeLHDR(dev, layer);
    for (const CellSpan &span : layer.spans) {
      writeCHDR(dev, span);
      if (span.cell) {
        writeCDAT(dev, span.cell->image, canvasFormat);
      }
    }
  }
}

void Timeline::serializeTail(QIODevice &dev) const {
  ChunkWriter writer{dev};
  writer.begin(0, "AEND");
  writer.end();
}

void Timeline::deserialize(QIODevice *dev) {
  assert(dev);
  layers.resize(deserializeBytesAs<uint16_t>(dev));
  frameCount = FrameIdx{deserializeBytesAs<uint16_t>(dev)};
  for (Layer &layer : layers) {
    deserializeBytes(dev, layer.visible);
    layer.name.resize(deserializeBytesAs<uint16_t>(dev));
    dev->read(layer.name.data(), layer.name.size());
    layer.spans.resize(deserializeBytesAs<uint16_t>(dev));
    for (CellSpan &span : layer.spans) {
      span.len = FrameIdx{deserializeBytesAs<uint16_t>(dev)};
      if (deserializeBytesAs<bool>(dev)) {
        span.cell = makeCell();
        deserializeImage(dev, span.cell->image);
      }
    }
  }
  selection = empty_rect;
  changeFrameCount();
  changeLayerCount();
  changeFrame();
  changePos();
  Q_EMIT selectionChanged(selection);
  changeLayers(LayerIdx{0}, layerCount());
}

CellRect Timeline::selectCells(const ExportOptions &options) const {
  CellRect rect;
  switch (options.layerSelect) {
    case LayerSelect::all_composited:
    case LayerSelect::all:
      rect.minL = LayerIdx{0};
      rect.maxL = layerCount() - LayerIdx{1};
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
  return rect;
}

namespace {

inline void assertEval([[maybe_unused]] const bool cond) noexcept {
  assert(cond);
}

QImage grayToIndexed(const PaletteCSpan palette, QImage image) {
  assert(image.format() == QImage::Format_Grayscale8);
  assertEval(image.reinterpretAsFormat(QImage::Format_Indexed8));
  QVector<QRgb> table(static_cast<int>(palette.size()));
  std::copy(palette.cbegin(), palette.cend(), table.begin());
  image.setColorTable(table);
  return image;
}

QImage grayToMono(const QImage &src) {
  QImage dst{src.size(), QImage::Format_Mono};
  for (int y = 0; y != dst.height(); ++y) {
    for (int x = 0; x != dst.width(); ++x) {
      const int gray = FormatGray::toGray(src.pixel(x, y));
      dst.setPixel(x, y, gray < 128 ? 0 : 1);
    }
  }
  return dst;
}

QImage grayWithoutAlpha(const QImage &src) {
  assert(src.format() == QImage::Format_Grayscale16);
  QImage dst{src.size(), QImage::Format_Grayscale8};
  pixelTransform(makeSurface<uint8_t>(dst), makeCSurface<PixelGray>(src), &FormatGray::toGray);
  return dst;
}

QImage convertIndexImage(const ExportFormat format, const PaletteCSpan palette, QImage image) {
  switch (format) {
    case ExportFormat::rgba:
      assert(image.format() == qimageFormat(Format::rgba));
      return image;
    case ExportFormat::index:
      return grayToIndexed(palette, image);
    case ExportFormat::gray:
      assert(image.format() == QImage::Format_Grayscale8);
      return image;
    case ExportFormat::monochrome:
      assert(image.format() == QImage::Format_Grayscale8);
      return grayToMono(image);
    default: Q_UNREACHABLE();
  }
}

QImage convertGrayImage(const ExportFormat format, QImage image) {
  switch (format) {
    case ExportFormat::gray:
      return grayWithoutAlpha(image);
    case ExportFormat::gray_alpha:
      // impossible without libpng
      // Qt doesn't support gray with alpha
      return image;
    case ExportFormat::monochrome:
      return grayToMono(image);
    default: Q_UNREACHABLE();
  }
}

}

QImage Timeline::convertImage(
  const ExportFormat format,
  const PaletteCSpan palette,
  QImage image
) const {
  // @TODO libpng
  switch (canvasFormat) {
    case Format::rgba:
      assert(format == ExportFormat::rgba);
      return image;
    case Format::index:
      return convertIndexImage(format, palette, image);
    case Format::gray:
      return convertGrayImage(format, image);
    default: Q_UNREACHABLE();
  }
}

namespace {

template <typename Idx>
Idx apply(const Line<Idx> line, const Idx value) {
  return value * line.stride + line.offset;
}

}

void Timeline::exportFile(
  const ExportOptions &options,
  const PaletteCSpan palette,
  QImage image,
  CellPos pos
) const {
  QString path = options.directory;
  if (path.back() != QDir::separator()) {
    path.push_back(QDir::separator());
  }
  pos.l = apply(options.layerLine, pos.l);
  pos.f = apply(options.frameLine, pos.f);
  path += evalExportPattern(options.name, pos.l, pos.f);
  path += ".png";
  assertEval(convertImage(options.format, palette, image).save(path));
}

void Timeline::exportCompRect(
  const ExportOptions &options,
  const PaletteCSpan palette,
  const CellRect rect
) const {
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
      if (const Cell *cell = *iterators[+l]; cell) {
        frame.push_back(cell);
      }
      ++iterators[+l];
    }
    QImage result = compositeFrame(palette, frame, canvasSize, canvasFormat);
    exportFile(options, palette, result, {rect.minL, f});
  }
}

void Timeline::exportRect(
  const ExportOptions &options,
  const PaletteCSpan palette,
  const CellRect rect
) const {
  for (LayerIdx l = rect.minL; l <= rect.maxL; ++l) {
    const Layer &layer = layers[+l];
    // @TODO does the user want to skip invisible layers?
    if (!layer.visible) continue;
    LayerCells::ConstIterator iter = layer.spans.find(rect.minF);
    for (FrameIdx f = rect.minF; f <= rect.maxF; ++f) {
      if (const Cell *cell = *iter; cell) {
        exportFile(options, palette, cell->image, {l, f});
      }
      ++iter;
    }
  }
}

void Timeline::exportTimeline(const ExportOptions &options, const PaletteCSpan palette) const {
  const CellRect rect = selectCells(options);
  if (composited(options.layerSelect)) {
    exportCompRect(options, palette, rect);
  } else {
    exportRect(options, palette, rect);
  }
}

void Timeline::initCanvas(const Format format, const QSize size) {
  canvasFormat = format;
  canvasSize = size;
}

void Timeline::nextFrame() {
  currPos.f = (currPos.f + FrameIdx{1}) % frameCount;
  changeFrame();
  changePos();
}

void Timeline::prevFrame() {
  currPos.f = (currPos.f - FrameIdx{1} + frameCount) % frameCount;
  changeFrame();
  changePos();
}

void Timeline::layerBelow() {
  currPos.l = std::min(currPos.l + LayerIdx{1}, layerCount() - LayerIdx{1});
  changePos();
}

void Timeline::layerAbove() {
  currPos.l = std::max(currPos.l - LayerIdx{1}, LayerIdx{0});
  changePos();
}

namespace {

CellRect normalize(const CellRect rect) {
  return {
    std::min(rect.minL, rect.maxL),
    std::min(rect.minF, rect.maxF),
    std::max(rect.minL, rect.maxL),
    std::max(rect.minF, rect.maxF)
  };
}

}

void Timeline::beginSelection() {
  selection.minL = currPos.l;
  selection.minF = currPos.f;
  selection.maxL = currPos.l;
  selection.maxF = currPos.f;
  Q_EMIT selectionChanged(selection);
}

void Timeline::continueSelection() {
  selection.maxL = currPos.l;
  selection.maxF = currPos.f;
  Q_EMIT selectionChanged(normalize(selection));
}

void Timeline::endSelection() {
  selection.maxL = currPos.l;
  selection.maxF = currPos.f;
  selection = normalize(selection);
  Q_EMIT selectionChanged(selection);
}

void Timeline::clearSelection() {
  selection = empty_rect;
  Q_EMIT selectionChanged(selection);
}

void Timeline::insertLayer() {
  Layer layer;
  layer.spans.pushNull(frameCount);
  layer.name = "Layer " + std::to_string(layers.size());
  layers.insert(layers.begin() + +currPos.l, std::move(layer));
  changeLayerCount();
  Q_EMIT selectionChanged(selection);
  changeLayers(currPos.l, layerCount());
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::removeLayer() {
  if (layers.size() == 1) {
    layers.front().spans.clear(frameCount);
    layers.front().name = "Layer 0";
    layers.front().visible = true;
    changeLayers(LayerIdx{0}, LayerIdx{1});
  } else {
    layers.erase(layers.begin() + +currPos.l);
    changeLayerCount();
    Q_EMIT selectionChanged(selection);
    currPos.l = std::min(currPos.l, layerCount() - LayerIdx{1});
    changeLayers(currPos.l, layerCount());
  }
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::moveLayerUp() {
  if (currPos.l == LayerIdx{0}) return;
  std::swap(layers[+(currPos.l - LayerIdx{1})], layers[+currPos.l]);
  changeLayers(currPos.l - LayerIdx{1}, currPos.l + LayerIdx{1});
  changeFrame();
  layerAbove();
  Q_EMIT modified();
}

void Timeline::moveLayerDown() {
  if (currPos.l == layerCount() - LayerIdx{1}) return;
  std::swap(layers[+currPos.l], layers[+(currPos.l + LayerIdx{1})]);
  changeLayers(currPos.l, currPos.l + LayerIdx{2});
  changeFrame();
  layerBelow();
  Q_EMIT modified();
}

void Timeline::insertFrame() {
  ++frameCount;
  changeFrameCount();
  for (LayerIdx l = {}; l != layerCount(); ++l) {
    layers[+l].spans.insertCopy(currPos.f);
    changeSpan(l);
  }
  Q_EMIT selectionChanged(selection);
  nextFrame();
  Q_EMIT modified();
}

void Timeline::insertNullFrame() {
  ++frameCount;
  changeFrameCount();
  for (LayerIdx l = {}; l != layerCount(); ++l) {
    layers[+l].spans.insertNew(currPos.f, nullptr);
    changeSpan(l);
  }
  Q_EMIT selectionChanged(selection);
  nextFrame();
  Q_EMIT modified();
}

void Timeline::removeFrame() {
  if (frameCount == FrameIdx{1}) {
    for (LayerIdx l = {}; l != layerCount(); ++l) {
      layers[+l].spans.clear(FrameIdx{1});
      changeSpan(l);
    }
  } else {
    --frameCount;
    changeFrameCount();
    for (LayerIdx l = {}; l != layerCount(); ++l) {
      layers[+l].spans.remove(currPos.f);
      changeSpan(l);
    }
    Q_EMIT selectionChanged(selection);
  }
  currPos.f = std::max(currPos.f - FrameIdx{1}, FrameIdx{0});
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::clearCell() {
  layers[+currPos.l].spans.replaceNew(currPos.f, nullptr);
  changeSpan(currPos.l);
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::extendCell() {
  layers[+currPos.l].spans.extend(currPos.f);
  changeSpan(currPos.l);
  nextFrame();
  Q_EMIT modified();
}

void Timeline::splitCell() {
  layers[+currPos.l].spans.split(currPos.f);
  changeSpan(currPos.l);
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::requestCell() {
  layers[+currPos.l].spans.replaceNew(currPos.f, makeCell());
  changeSpan(currPos.l);
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::setCurrPos(const CellPos pos) {
  assert(LayerIdx{0} <= pos.l);
  assert(pos.l < layerCount());
  assert(FrameIdx{0} <= pos.f);
  assert(pos.f < frameCount);
  if (currPos.f != pos.f) {
    currPos = pos;
    changeFrame();
    changePos();
  } else if (currPos.l != pos.l) {
    currPos.l = pos.l;
    changePos();
  }
}

void Timeline::setVisibility(const LayerIdx idx, const bool visible) {
  assert(LayerIdx{0} <= idx);
  assert(idx < layerCount());
  bool &layerVis = layers[+idx].visible;
  // @TODO Emit signal when layer visibility changed?
  // if (layerVis != visible) {
    layerVis = visible;
  //   Q_EMIT visibilityChanged(idx, visible);
  // }
  changeFrame();
  Q_EMIT modified();
}

void Timeline::setName(const LayerIdx idx, const std::string_view name) {
  assert(LayerIdx{0} <= idx);
  assert(idx < layerCount());
  layers[+idx].name = name;
  // @TODO Emit signal when layer name changed?
  // Q_EMIT nameChanged(idx, name);
  Q_EMIT modified();
}

void Timeline::clearSelected() {
  LayerCells nullSpans;
  nullSpans.pushNull(selection.maxF - selection.minF + FrameIdx{1});
  for (LayerIdx l = selection.minL; l <= selection.maxL; ++l) {
    layers[+l].spans.replaceSpan(selection.minF, nullSpans);
    changeSpan(l);
  }
  changeFrame();
  changePos();
  Q_EMIT modified();
}

void Timeline::copySelected() {
  clipboard.clear();
  for (LayerIdx l = selection.minL; l <= selection.maxL; ++l) {
    const FrameIdx idx = selection.minF;
    const FrameIdx len = selection.maxF - selection.minF + FrameIdx{1};
    clipboard.push_back(layers[+l].spans.extract(idx, len));
  }
}

void Timeline::pasteSelected() {
  if (selection.minL > selection.maxL) return;
  const LayerIdx endLayer = std::min(
    layerCount(), selection.minL + static_cast<LayerIdx>(clipboard.size())
  );
  const FrameIdx frames = frameCount - selection.minF;
  for (LayerIdx l = selection.minL; l < endLayer; ++l) {
    LayerCells spans = clipboard[+(l - selection.minL)].truncateCopy(frames);
    layers[+l].spans.replaceSpan(selection.minF, spans);
    changeSpan(l);
  }
  changeFrame();
  changePos();
  Q_EMIT modified();
}

CellPtr Timeline::makeCell() const {
  return std::make_unique<Cell>(canvasSize, canvasFormat);
}

Cell *Timeline::getCell(const CellPos pos) {
  return layers[+pos.l].spans.get(pos.f);
}

Frame Timeline::getFrame(const FrameIdx pos) const {
  Frame frame;
  frame.reserve(layers.size());
  for (const Layer &layer : layers) {
    if (layer.visible) {
      if (const Cell *cell = layer.spans.get(pos); cell) {
        frame.push_back(cell);
      }
    }
  }
  return frame;
}

LayerIdx Timeline::layerCount() const {
  return static_cast<LayerIdx>(layers.size());
}

void Timeline::changePos() {
  Q_EMIT currPosChanged(currPos);
  Q_EMIT currCellChanged(getCell(currPos));
}

void Timeline::changeFrame() {
  Q_EMIT frameChanged(getFrame(currPos.f));
}

void Timeline::changeSpan(const LayerIdx idx) {
  Q_EMIT layerChanged(idx, layers[+idx].spans);
}

void Timeline::changeLayers(const LayerIdx begin, const LayerIdx end) {
  assert(begin < end);
  for (LayerIdx l = begin; l != end; ++l) {
    Q_EMIT layerChanged(l, layers[+l].spans);
    Q_EMIT visibilityChanged(l, layers[+l].visible);
    Q_EMIT nameChanged(l, layers[+l].name);
  }
}

void Timeline::changeFrameCount() {
  Q_EMIT frameCountChanged(frameCount);
}

void Timeline::changeLayerCount() {
  Q_EMIT layerCountChanged(layerCount());
}

#include "timeline.moc"
