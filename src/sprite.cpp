//
//  sprite.cpp
//  Animera
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "sprite.hpp"

#include "file io.hpp"
#include "sprite file.hpp"

void Sprite::newFile(const Format newFormat, const QSize newSize) {
  format = newFormat;
  size = newSize;
  Q_EMIT canvasInitialized(format, size);
  palette.initCanvas(format);
  timeline.initCanvas(format, size);
  palette.initDefault();
  timeline.initDefault();
}

namespace {

/*

AHDR
animation header
 - width
 - height
 - number of layers
 - number of frames
 - delay
 - format

PLTE
palette
 - colors excluding trailing 0 entries

LHDR
layer header
 - number of spans in the layer
 - byte visibility
 - ascii string name

CHDR
cell header
 - length of span
 - x coord
 - y coord
 - width
 - height

CDAT
cell data
 - compressed image data
 
AEND
animation end
*/

}

void Sprite::optimize() {
  timeline.optimize();
}

Error Sprite::saveFile(const QString &path) const {
  FileWriter writer;
  if (Error err = writer.open(path); err) return err;
  if (Error err = writeSignature(writer.dev()); err) return err;
  if (Error err = timeline.serializeHead(writer.dev()); err) return err;
  if (Error err = palette.serialize(writer.dev()); err) return err;
  if (Error err = timeline.serializeBody(writer.dev()); err) return err;
  if (Error err = timeline.serializeTail(writer.dev()); err) return err;
  return writer.flush();
}

Error Sprite::openFile(const QString &path) {
  FileReader reader;
  if (Error err = reader.open(path); err) return err;
  if (Error err = readSignature(reader.dev()); err) return err;
  if (Error err = timeline.deserializeHead(reader.dev(), format, size); err) return err;
  Q_EMIT canvasInitialized(format, size);
  palette.initCanvas(format);
  timeline.initCanvas(format, size);
  if (Error err = palette.deserialize(reader.dev()); err) return err;
  if (Error err = timeline.deserializeBody(reader.dev()); err) return err;
  if (Error err = timeline.deserializeTail(reader.dev()); err) return err;
  return reader.flush();
}

Error Sprite::openImage(const QString &path) {
  if (Error err = timeline.openImage(path, palette.getPalette(), format, size); err) return err;
  Q_EMIT canvasInitialized(format, size);
  palette.initCanvas(format);
  timeline.initCanvas(format, size);
  if (format != Format::index) {
    palette.reset();
  }
  palette.change();
  timeline.change();
  return {};
}

Error Sprite::exportSprite(const ExportOptions &options) const {
  return timeline.exportTimeline(options, palette.getPalette());
}

Format Sprite::getFormat() const {
  return format;
}

QSize Sprite::getSize() const {
  return size;
}

#include "sprite.moc"
