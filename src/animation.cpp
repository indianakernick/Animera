//
//  animation.cpp
//  Animera
//
//  Created by Indiana Kernick on 7/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "animation.hpp"

#include "file io.hpp"
#include "scope time.hpp"
#include "animation file.hpp"

void Animation::optimize() {
  timeline.optimize();
}

void Animation::resizeCanvas(const QSize newSize) {
  if (size == newSize) return;
  size = newSize;
  timeline.resizeCanvas(size);
  Q_EMIT canvasResized(size);
}

void Animation::newFile(const Format newFormat, const QSize newSize) {
  SCOPE_TIME("Animation::newFile");
  
  format = newFormat;
  size = newSize;
  Q_EMIT canvasInitialized(format, size);
  palette.initCanvas(format);
  timeline.initCanvas(format, size);
  palette.initDefault();
  timeline.initDefault();
}

Error Animation::saveFile(const QString &path) const {
  SCOPE_TIME("Animation::saveFile");

  FileWriter writer;
  TRY(writer.open(path));
  TRY(writeSignature(writer.dev()));
  TRY(timeline.serializeHead(writer.dev()));
  TRY(palette.serialize(writer.dev()));
  TRY(timeline.serializeBody(writer.dev()));
  TRY(timeline.serializeTail(writer.dev()));
  return writer.flush();
}

Error Animation::openFile(const QString &path) {
  SCOPE_TIME("Animation::openFile");
  
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

Error Animation::openImage(const QString &path) {
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

Format Animation::getFormat() const {
  return format;
}

QSize Animation::getSize() const {
  return size;
}

#include "animation.moc"
