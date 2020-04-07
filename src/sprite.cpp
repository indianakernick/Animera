//
//  sprite.cpp
//  Animera
//
//  Created by Indiana Kernick on 7/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "sprite.hpp"

#include "file io.hpp"
#include "scope time.hpp"
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

void Sprite::optimize() {
  timeline.optimize();
}

Error Sprite::saveFile(const QString &path) const {
  SCOPE_TIME("Sprite::saveFile");

  FileWriter writer;
  TRY(writer.open(path));
  TRY(writeSignature(writer.dev()));
  TRY(timeline.serializeHead(writer.dev()));
  TRY(palette.serialize(writer.dev()));
  TRY(timeline.serializeBody(writer.dev()));
  TRY(timeline.serializeTail(writer.dev()));
  return writer.flush();
}

Error Sprite::openFile(const QString &path) {
  SCOPE_TIME("Sprite::openFile");
  
  FileReader reader;
  TRY(reader.open(path));
  TRY(readSignature(reader.dev()));
  TRY(timeline.deserializeHead(reader.dev(), format, size));
  Q_EMIT canvasInitialized(format, size);
  palette.initCanvas(format);
  timeline.initCanvas(format, size);
  TRY(palette.deserialize(reader.dev()));
  TRY(timeline.deserializeBody(reader.dev()));
  TRY(timeline.deserializeTail(reader.dev()));
  return reader.flush();
}

Error Sprite::openImage(const QString &path) {
  TRY(timeline.openImage(path, palette.getPalette(), format, size));
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
