//
//  json atlas generator.cpp
//  Animera
//
//  Created by Indiana Kernick on 27/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "json atlas generator.hpp"

#include "file io.hpp"
#include <QtCore/qdir.h>

JsonAtlasGenerator::JsonAtlasGenerator()
  : packer{DataFormat::png} {}

Error JsonAtlasGenerator::initAtlas(const PixelFormat format, const QString &name, const QString &dir) {
  if (format == PixelFormat::index) {
    return "JSON Atlas Generator does not support indexed pixel format";
  }
  if (format == PixelFormat::monochrome) {
    return "JSON Atlas Generator does not support monochrome pixel format";
  }
  
  packer.init(format);
  atlas = "{\"names\":{\"null_\":0";
  atlasName = name;
  atlasDir = dir;
  names.clear();
  collision.clear();
  return {};
}

namespace {

void escapeJsonString(QString &str) {
  for (int i = 0; i != str.size(); ++i) {
    if (str[i] == '\\') {
      str.insert(i, '\\');
      ++i;
    } else if (str[i] == '\"') {
      str.insert(i, '\\');
      ++i;
    }
  }
}

}

void JsonAtlasGenerator::addName(
  const std::size_t i,
  const SpriteNameParams &params,
  const SpriteNameState &state
) {
  // TODO: How do we reuse CppAtlasGenerator::addName?
  QString name = evaluateSpriteName(params, state);
  insertName(name);
  escapeJsonString(name);
  appendName(name, i);
}

void JsonAtlasGenerator::addSize(const QSize size) {
  packer.append(size);
}

void JsonAtlasGenerator::addWhiteName() {
  appendName("whitepixel_", packer.count());
  insertName("whitepixel_");
  packer.appendWhite();
}

QString JsonAtlasGenerator::hasNameCollision() {
  return collision;
}

Error JsonAtlasGenerator::packRectangles() {
  TRY(packer.pack());
  atlas += "},\"rects\":[[0,0,0,0]";
  return {};
}

Error JsonAtlasGenerator::initAnimation(const Format format, const PaletteCSpan palette) {
  return packer.setFormat(format, palette);
}

Error JsonAtlasGenerator::addImage(const std::size_t i, const QImage &img) {
  if (img.isNull()) {
    appendRect({});
  } else {
    appendRect(packer.copy(i, img));
  }
  return {};
}

Error JsonAtlasGenerator::addWhiteImage() {
  appendRect(packer.copyWhite(packer.count() - 1));
  return {};
}

Error JsonAtlasGenerator::finalize() {
  atlas += "],\"count\":";
  atlas += QString::number(packer.count() + 1);
  atlas += ",\"width\":";
  atlas += QString::number(packer.width());
  atlas += ",\"height\":";
  atlas += QString::number(packer.height());
  atlas += "}\n";
  
  FileWriter writer;
  TRY(writer.open(atlasDir + QDir::separator() + atlasName + ".json"));
  writer.dev().setTextModeEnabled(true);
  const QByteArray utf8 = atlas.toUtf8();
  if (writer.dev().write(utf8) != utf8.size()) {
    return "Error writing atlas\n";
  }
  TRY(writer.flush());
  
  TRY(writer.open(atlasDir + QDir::separator() + atlasName + ".png"));
  TRY(packer.write(writer.dev()));
  TRY(writer.flush());
  
  return {};
}

void JsonAtlasGenerator::insertName(const QString &name) {
  if (collision.isEmpty() && !names.insert(name).second) {
    collision = name;
  }
}

void JsonAtlasGenerator::appendName(const QString &name, const std::size_t i) {
  atlas += ",\"";
  atlas += name;
  atlas += "\":";
  atlas += QString::number(i + 1);
}

void JsonAtlasGenerator::appendRect(const QRect r) {
  if (r.isEmpty()) {
    atlas += ",[0,0,0,0]";
  } else {
    atlas += ",[";
    atlas += QString::number(r.x());
    atlas += ',';
    atlas += QString::number(r.y());
    atlas += ',';
    atlas += QString::number(r.width());
    atlas += ',';
    atlas += QString::number(r.height());
    atlas += ']';
  }
}
