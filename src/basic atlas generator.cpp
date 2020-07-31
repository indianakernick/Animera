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

bool BasicAtlasGenerator::supported(const PixelFormat pixelFormat, const Format format) const {
  switch (pixelFormat) {
    case PixelFormat::rgba:
      return true;
    case PixelFormat::index:
      return false;
    case PixelFormat::gray:
      return format == Format::gray;
    case PixelFormat::gray_alpha:
      return format == Format::gray;
    case PixelFormat::monochrome:
      return false;
  }
}

Error BasicAtlasGenerator::beginAtlas(const AtlasInfo &info) {
  packer.init(info.pixelFormat);
  names.clear();
  names.insert("null_");
  collision.clear();
  return {};
}

void BasicAtlasGenerator::appendName(const std::size_t i, const NameInfo info) {
  std::array<int, 4> namePos;
  QString name = info.params.baseName;
  namePos[0] = name.size();
  appendLayerName(name, info.params, info.state);
  namePos[1] = name.size();
  appendGroupName(name, info.params, info.state);
  namePos[2] = name.size();
  appendFrameName(name, info.params, info.state);
  namePos[3] = name.size();
  
  fixName(name, namePos);
  
  const bool hasLayerName = namePos[1] > namePos[0] && namePos[0] > 0;
  const bool hasGroupName = namePos[2] > namePos[1] && namePos[1] > 0;
  const bool hasFrameName = namePos[3] > namePos[2] && namePos[2] > 0;
  
  if (info.state.frame == info.state.groupBegin) {
    if (info.state.group == GroupIdx{0}) {
      if (hasLayerName && info.state.layer == LayerIdx{0}) {
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
  
  if (info.state.frame - info.state.groupBegin == info.state.frameCount - FrameIdx{1}) {
    if (hasFrameName) {
      appendAlias(name.left(namePos[2]), "end_", i + 1);
    }
    if (info.state.group == info.state.groupCount - GroupIdx{1}) {
      if (hasGroupName) {
        appendAlias(name.left(namePos[1]), "end_", i + 1);
      }
      if (hasLayerName && info.state.layer == info.state.layerCount - LayerIdx{1}) {
        appendAlias(name.left(namePos[0]), "end_", i + 1);
      }
    }
  }
  
  packer.append(info.size);
}

void BasicAtlasGenerator::appendWhiteName(const std::size_t i) {
  appendName("whitepixel_", i);
  insertName("whitepixel_");
  packer.appendWhite();
}

QString BasicAtlasGenerator::endNames() {
  return collision;
}

Error BasicAtlasGenerator::beginImages() {
  return packer.pack();
}

Error BasicAtlasGenerator::setImageFormat(const Format format, const PaletteCSpan palette) {
  return packer.setFormat(format, palette);
}

Error BasicAtlasGenerator::copyImage(const std::size_t i, const QImage &img) {
  if (img.isNull()) {
    appendRect({});
  } else {
    appendRect(packer.copy(i, img));
  }
  return {};
}

Error BasicAtlasGenerator::copyWhiteImage(const std::size_t i) {
  appendRect(packer.copyWhite(i));
  return {};
}

void BasicAtlasGenerator::insertName(const QString &name) {
  if (collision.isEmpty() && !names.insert(name).second) {
    collision = name;
  }
}
