//
//  basic atlas generator.cpp
//  Animera
//
//  Created by Indiana Kernick on 28/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "basic atlas generator.hpp"

BasicAtlasGenerator::BasicAtlasGenerator(const DataFormat format)
  : packer{format} {}

Error BasicAtlasGenerator::initAtlas(const PixelFormat format, const QString &, const QString &) {
  if (format == PixelFormat::index) {
    return "Indexed pixel format is not supported by this generator";
  }
  if (format == PixelFormat::monochrome) {
    return "Monochrome pixel format is not supported by this generator";
  }
  
  packer.init(format);
  names.clear();
  names.insert("null_");
  collision.clear();
  return {};
}

void BasicAtlasGenerator::addName(
  const std::size_t i,
  const SpriteNameParams &params,
  const SpriteNameState &state
) {
  std::array<int, 4> namePos;
  QString name = params.baseName;
  namePos[0] = name.size();
  appendLayerName(name, params, state);
  namePos[1] = name.size();
  appendGroupName(name, params, state);
  namePos[2] = name.size();
  appendFrameName(name, params, state);
  namePos[3] = name.size();
  
  fixName(name, namePos);
  
  const bool hasLayerName = namePos[1] > namePos[0] && namePos[0] > 0;
  const bool hasGroupName = namePos[2] > namePos[1] && namePos[1] > 0;
  const bool hasFrameName = namePos[3] > namePos[2] && namePos[2] > 0;
  
  if (state.frame == state.groupBegin) {
    if (state.group == GroupIdx{0}) {
      if (hasLayerName && state.layer == LayerIdx{0}) {
        appendAlias(name.left(namePos[0]), "beg_", i);
      }
      if (hasGroupName) {
        appendAlias(name.left(namePos[1]), "beg_", i);
      }
    }
    if (hasFrameName) {
      appendAlias(name.left(namePos[2]), "beg_", i);
    }
  }
  
  appendName(name, i);
  insertName(name);
  
  if (state.frame - state.groupBegin == state.frameCount - FrameIdx{1}) {
    if (hasFrameName) {
      appendAlias(name.left(namePos[2]), "end_", i + 1);
    }
    if (state.group == state.groupCount - GroupIdx{1}) {
      if (hasGroupName) {
        appendAlias(name.left(namePos[1]), "end_", i + 1);
      }
      if (hasLayerName && state.layer == state.layerCount - LayerIdx{1}) {
        appendAlias(name.left(namePos[0]), "end_", i + 1);
      }
    }
  }
}

void BasicAtlasGenerator::addSize(const QSize size) {
  packer.append(size);
}

void BasicAtlasGenerator::addWhiteName() {
  appendName("whitepixel_", packer.count());
  insertName("whitepixel_");
  packer.appendWhite();
}

QString BasicAtlasGenerator::hasNameCollision() {
  return collision;
}

Error BasicAtlasGenerator::packRectangles() {
  return packer.pack();
}

Error BasicAtlasGenerator::initAnimation(const Format format, const PaletteCSpan palette) {
  return packer.setFormat(format, palette);
}

Error BasicAtlasGenerator::addImage(const std::size_t i, const QImage &img) {
  if (img.isNull()) {
    appendRect({});
  } else {
    appendRect(packer.copy(i, img));
  }
  return {};
}

Error BasicAtlasGenerator::addWhiteImage() {
  appendRect(packer.copyWhite(packer.count() - 1));
  return {};
}

void BasicAtlasGenerator::insertName(const QString &name) {
  if (collision.isEmpty() && !names.insert(name).second) {
    collision = name;
  }
}
