//
//  export texture atlas.cpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "export texture atlas.hpp"

#include "sprite.hpp"
#include "composite.hpp"
#include "export backend.hpp"
#include "surface factory.hpp"
#include "graphics convert.hpp"
#include <Graphics/transform.hpp>

namespace {

bool includeLayer(const LayerVis vis, const bool visible) {
  switch (vis) {
    case LayerVis::visible: return visible;
    case LayerVis::hidden: return !visible;
    case LayerVis::all: return true;
  }
}

template <typename Func>
Error eachFrame(const SpriteExportParams &params, const Sprite &sprite, Func func) {
  const tcb::span<const Layer> layers = sprite.timeline.getLayerArray();
  const LayerRange layerRange = params.layers;
  const FrameRange frameRange = params.frames;
  const LayerIdx layerCount = layerRange.max - layerRange.min + LayerIdx{1};
  
  Frame frame;
  frame.reserve(+layerCount);
  
  std::vector<LayerCels::ConstIterator> celIters;
  celIters.reserve(+layerCount);
  for (LayerIdx l = layerRange.min; l <= layerRange.max; ++l) {
    celIters.push_back(layers[+l].spans.find(frameRange.min));
  }
  GroupIterator groupIter{sprite.timeline.getGroupArray(), frameRange.min};
  
  ExportNameState state;
  state.layerCount = LayerIdx{1};
  state.groupCount = sprite.timeline.getGroups();
  
  for (FrameIdx f = frameRange.min; f <= frameRange.max; ++f) {
    frame.clear();
    for (LayerIdx l = {}; l != layerCount; ++l) {
      if (!includeLayer(layerRange.vis, layers[+(l + layerRange.min)].visible)) continue;
      if (const Cel *cel = *celIters[+l]; *cel) {
        frame.push_back(cel);
      }
      ++celIters[+l];
    }
    
    const GroupInfo info = groupIter.info();
    state.layer = layerRange.min;
    state.group = info.group;
    state.frame = f - info.begin;
    state.frameCount = info.end - info.begin;
    state.groupBegin = info.begin;
    state.layerName = layers[+layerRange.min].name;
    state.groupName = groupIter.name();
    ++groupIter;
    
    TRY(func(frame, state));
  }
  
  return {};
}

template <typename Func>
Error eachCel(const SpriteExportParams &params, const Sprite &sprite, Func func) {
  // constexpr bool calculate_state = std::is_invocable_v<Func, const Cel *, const ExportNameState &>;
  
  const tcb::span<const Layer> layers = sprite.timeline.getLayerArray();
  const LayerRange layerRange = params.layers;
  const FrameRange frameRange = params.frames;
  
  ExportNameState state;
  state.layerCount = sprite.timeline.getLayers();
  state.groupCount = sprite.timeline.getGroups();
  
  for (LayerIdx l = layerRange.min; l <= layerRange.max; ++l) {
    const Layer &layer = layers[+l];
    if (!includeLayer(layerRange.vis, layer.visible)) continue;
    auto celIter = layer.spans.find(frameRange.min);
    GroupIterator groupIter{sprite.timeline.getGroupArray(), frameRange.min};
    
    state.layer = l;
    state.layerName = layer.name;
    
    for (FrameIdx f = frameRange.min; f <= frameRange.max; ++f) {
      if (const Cel *cel = *celIter; *cel) {
        const GroupInfo info = groupIter.info();
        state.group = info.group;
        state.frame = f - info.begin;
        state.frameCount = info.end - info.begin;
        state.groupBegin = info.begin;
        state.groupName = groupIter.name();
        TRY(func(cel, state));
      }
      ++celIter;
      ++groupIter;
    }
  }
  
  return {};
}

struct Images {
  QImage canvas;
  QImage xformed;
};

QSize getTransformedSize(const QSize canvasSize, const ExportTransform &transform) {
  if (transform.scaleX == 1 && transform.scaleY == 1 && transform.angle == 0) {
    return {};
  }
  QSize size;
  size.setWidth(canvasSize.width() * std::abs(transform.scaleX));
  size.setHeight(canvasSize.height() * std::abs(transform.scaleY));
  return convert(gfx::rotateSize(convert(size), transform.angle));
}

void initImages(Images &images, const SpriteExportParams &params, const Sprite &sprite) {
  Format imageFormat = sprite.getFormat();
  if (params.composite && sprite.getFormat() != Format::gray) {
    imageFormat = Format::rgba;
  }
  images.canvas = {sprite.getSize(), qimageFormat(imageFormat)};
  const QSize xformedSize = getTransformedSize(images.canvas.size(), params.transform);
  images.xformed = {xformedSize, images.canvas.format()};
}

void applyTransform(Images &images, const ExportTransform &transform) {
  visitSurface(images.xformed, [&](const auto dst) {
    const auto src = makeCSurface<typename decltype(dst)::Pixel>(images.canvas);
    gfx::spatialTransform(dst, src, [&](const gfx::Point dstPos) {
      gfx::Point srcPos = gfx::rotate(transform.angle, dst.size(), dstPos);
      srcPos = transform.scaleX < 0 ? gfx::flipHori(dst.size(), srcPos) : srcPos;
      srcPos = transform.scaleY < 0 ? gfx::flipVert(dst.size(), srcPos) : srcPos;
      return gfx::scale({std::abs(transform.scaleX), std::abs(transform.scaleY)}, srcPos);
    });
  });
}

Error addImage(
  const std::size_t index,
  const ExportParams &params,
  const SpriteExportParams &spriteParams,
  Images &images
) {
  if (images.xformed.isNull()) {
    return params.backend->addImage(index, images.canvas);
  } else {
    applyTransform(images, spriteParams.transform);
    return params.backend->addImage(index, images.xformed);
  }
}

void addFrameNames(
  std::size_t &index,
  const ExportParams &params,
  const SpriteExportParams &spriteParams,
  const Sprite &sprite
) {
  auto iterate = [&](const Frame &, const ExportNameState &state) {
    params.backend->addName(index++, spriteParams.name, state);
    return Error{};
  };
  static_cast<void>(eachFrame(spriteParams, sprite, iterate));
}

void addCelNames(
  std::size_t &index,
  const ExportParams &params,
  const SpriteExportParams &spriteParams,
  const Sprite &sprite
) {
  auto iterate = [&](const Cel *, const ExportNameState &state) {
    params.backend->addName(index++, spriteParams.name, state);
    return Error{};
  };
  static_cast<void>(eachCel(spriteParams, sprite, iterate));
}

Error addFrameImages(
  std::size_t &index,
  const ExportParams &params,
  const SpriteExportParams &spriteParams,
  const Sprite &sprite
) {
  Images images;
  initImages(images, spriteParams, sprite);
  const Format format = sprite.getFormat();
  const PaletteCSpan palette = sprite.palette.getPalette();
  
  auto iterate = [&](const Frame &frame, const ExportNameState &) {
    if (format == Format::gray) {
      compositeFrame<FmtGray>(images.canvas, palette, frame, format, images.canvas.rect());
    } else {
      compositeFrame<FmtRgba>(images.canvas, palette, frame, format, images.canvas.rect());
    }
    return addImage(index++, params, spriteParams, images);
  };
  
  return eachFrame(spriteParams, sprite, iterate);
}

Error addCelImages(
  std::size_t &index,
  const ExportParams &params,
  const SpriteExportParams &spriteParams,
  const Sprite &sprite
) {
  Images images;
  initImages(images, spriteParams, sprite);
  
  auto iterate = [&](const Cel *cel, const ExportNameState &) {
    clearImage(images.canvas);
    blitImage(images.canvas, cel->img, cel->pos);
    return addImage(index++, params, spriteParams, images);
  };
  
  return eachCel(spriteParams, sprite, iterate);
}

bool compatibleFormat(
  const Format format,
  const bool composite,
  const PixelFormat pixelFormat
) {
  switch (format) {
    case Format::rgba:
      return pixelFormat == PixelFormat::rgba;
    case Format::index:
      if (composite) {
        return pixelFormat == PixelFormat::rgba;
      } else {
        return pixelFormat == PixelFormat::index ||
               pixelFormat == PixelFormat::gray ||
               pixelFormat == PixelFormat::monochrome;
      }
    case Format::gray:
      return pixelFormat == PixelFormat::gray_alpha ||
             pixelFormat == PixelFormat::gray ||
             pixelFormat == PixelFormat::monochrome;
  }
}

using SpritePtr = std::unique_ptr<const Sprite, void(*)(const Sprite *)>;
using SpriteArray = std::vector<SpritePtr>;

Error exportTextureAtlas(const ExportParams &params, const SpriteArray &sprites) {
  assert(params.backend);
  assert(params.sprites.size() == sprites.size());
  assert(!sprites.empty());
  
  for (std::size_t s = 0; s != sprites.size(); ++s) {
    if (!compatibleFormat(sprites[s]->getFormat(), params.sprites[s].composite, params.pixelFormat)) {
      return "Pixel format is not compatible with sprite format";
    }
  }
  
  TRY(params.backend->initAtlas(params.pixelFormat, params.name, params.directory));
  
  if (params.whitepixel) {
    params.backend->addWhiteName();
  }
  
  std::size_t spriteIndex = 0;
  for (std::size_t s = 0; s != sprites.size(); ++s) {
    std::size_t indexBefore = spriteIndex;
    if (params.sprites[s].composite) {
      addFrameNames(spriteIndex, params, params.sprites[s], *sprites[s]);
    } else {
      addCelNames(spriteIndex, params, params.sprites[s], *sprites[s]);
    }
    params.backend->addSizes(
      spriteIndex - indexBefore,
      getTransformedSize(sprites[s]->getSize(), params.sprites[s].transform)
    );
  }
  
  if (QString name = params.backend->hasNameCollision(); !name.isNull()) {
    return "Sprite name collision \"" + name + "\"";
  }
  
  if (params.whitepixel) {
    TRY(params.backend->addWhiteImage());
  }
  
  spriteIndex = 0;
  for (std::size_t s = 0; s != sprites.size(); ++s) {
    TRY(params.backend->initSprite(
      sprites[s]->getFormat(),
      sprites[s]->palette.getPalette()
    ));
    if (params.sprites[s].composite) {
      TRY(addFrameImages(spriteIndex, params, params.sprites[s], *sprites[s]));
    } else {
      TRY(addCelImages(spriteIndex, params, params.sprites[s], *sprites[s]));
    }
  }
  
  return params.backend->finalize();
}

}

Error exportTextureAtlas(const ExportParams &params, const std::vector<QString> &paths) {
  // TODO: This is not very efficient
  // We only need to load the selected portion of the file
  // Although the common case is to export the whole thing so
  // we'd be optimizing an uncommon situation
  
  // Also, I have to do all this nonsense instead of using std::vector<Sprite>
  // because QObject doesn't have a move constructor. Might be better off using
  // a simpler data structure here. We don't need the full functionality of
  // Sprite.
  SpriteArray sprites;
  for (const QString &path : paths) {
    auto *sprite = new Sprite;
    if (Error err = sprite->openFile(path)) {
      delete sprite;
      return err;
    }
    sprites.push_back(SpritePtr{sprite, [](const Sprite *sprite) {
      delete sprite;
    }});
  }
  
  return exportTextureAtlas(params, sprites);
}

Error exportTextureAtlas(const ExportParams &params, const Sprite &sprite) {
  SpriteArray sprites;
  sprites.push_back(SpritePtr{&sprite, [](const Sprite *) {}});
  return exportTextureAtlas(params, sprites);
}
