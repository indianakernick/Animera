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
  : BasicAtlasGenerator{DataFormat::png} {}

Error JsonAtlasGenerator::beginAtlas(const AtlasInfo &info) {
  TRY(BasicAtlasGenerator::beginAtlas(info));
  atlas = "{\"names\":{\"null_\":0";
  atlasName = info.name;
  atlasDir = info.directory;
  return {};
}

Error JsonAtlasGenerator::beginImages() {
  TRY(BasicAtlasGenerator::beginImages());
  atlas += "},\"rects\":[[0,0,0,0]";
  return {};
}

Error JsonAtlasGenerator::endAtlas() {
  atlas += "],\"width\":";
  atlas += QString::number(packer.width());
  atlas += ",\"height\":";
  atlas += QString::number(packer.height());
  atlas += "}\n";
  
  FileWriter writer;
  TRY(writer.open(atlasDir + '/' + atlasName + ".json"));
  writer.dev().setTextModeEnabled(true);
  const QByteArray utf8 = atlas.toUtf8();
  if (writer.dev().write(utf8) != utf8.size()) {
    return "Error writing atlas\n";
  }
  TRY(writer.flush());
  
  TRY(writer.open(atlasDir + '/' + atlasName + ".png"));
  TRY(packer.write(writer.dev()));
  TRY(writer.flush());
  
  return {};
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

void JsonAtlasGenerator::fixName(QString &name, std::array<int, 4> &positions) {
  for (int i = 0; i != name.size(); ++i) {
    if (name[i] == '\\') {
      name.insert(i, '\\');
    } else if (name[i] == '\"') {
      name.insert(i, '\\');
    } else continue;
    for (int &pos : positions) {
      if (i < pos) ++pos;
    }
    ++i;
  }
}

void JsonAtlasGenerator::appendAlias(QString base, const char *alias, const std::size_t i) {
  if (!base.isEmpty()) base += ' ';
  base += alias;
  appendName(base, i);
  insertName(base);
}
