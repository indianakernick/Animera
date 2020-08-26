//
//  export texture atlas.cpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "export texture atlas.hpp"

#include "animation.hpp"
#include "composite.hpp"
#include "atlas generator.hpp"
#include "surface factory.hpp"
#include "graphics convert.hpp"
#include <Graphics/transform.hpp>
#include "export sprite sheet.hpp"

namespace {

bool includeLayer(const LayerVis vis, const bool visible) {
  switch (vis) {
    case LayerVis::visible: return visible;
    case LayerVis::hidden: return !visible;
    case LayerVis::all: return true;
  }
}

template <typename Idx>
void checkNegative(Idx &idx, const Idx count) {
  if (idx < Idx{0}) idx += count;
}

Error validateRange(LayerRange &range, const LayerIdx count) {
  checkNegative(range.min, count);
  checkNegative(range.max, count);
  if (range.min < LayerIdx{0} || range.min >= count) {
    return "Layer min is out of range";
  }
  if (range.max < LayerIdx{0} || range.max >= count) {
    return "Layer max is out of range";
  }
  if (range.min > range.max) {
    return "Layer range is invalid";
  }
  return {};
}

Error validateRange(FrameRange &range, const FrameIdx count) {
  checkNegative(range.min, count);
  checkNegative(range.max, count);
  if (range.min < FrameIdx{0} || range.min >= count) {
    return "Frame min is out of range";
  }
  if (range.max < FrameIdx{0} || range.max >= count) {
    return "Frame max is out of range";
  }
  if (range.min > range.max) {
    return "Frame range is invalid";
  }
  return {};
}

Error validateSheetRange(
  const LayerRange layerRange,
  const FrameRange frameRange,
  const SpriteNameParams &params,
  const Timeline &timeline
) {
  const FrameIdx lastFrame = timeline.getFrames() - FrameIdx{1};
  const LayerIdx lastLayer = timeline.getLayers() - LayerIdx{1};
  
  if (isSheetMode(params.layerName)) {
    if (frameRange.min != FrameIdx{0} || frameRange.max != lastFrame) {
      return "Frame range is not aligned with sprite sheet";
    }
    if (layerRange.min != LayerIdx{0} || layerRange.max != lastLayer) {
      return "Layer range is not aligned with sprite sheet";
    }
    
    const tcb::span<const Layer> layers = timeline.getLayerArray();
    switch (layerRange.vis) {
      case LayerVis::visible:
        if (!layers.front().visible || !layers.back().visible) {
          return "Layer visibility is not aligned with sprite sheet";
        }
        break;
      case LayerVis::hidden:
        if (layers.front().visible || layers.back().visible) {
          return "Layer visibility is not aligned with sprite sheet";
        }
        break;
      case LayerVis::all:
        break;
    }
    return {};
  } else if (isSheetMode(params.groupName)) {
    if (frameRange.min != FrameIdx{0} || frameRange.max != lastFrame) {
      return "Frame range is not aligned with sprite sheet";
    }
  } else if (isSheetMode(params.frameName)) {
    const tcb::span<const Group> groups = timeline.getGroupArray();
    if (frameRange.min != FrameIdx{0} && !findGroupBoundary(groups, frameRange.min - FrameIdx{1})) {
      return "Frame range is not aligned with sprite sheet";
    }
    if (frameRange.max != lastFrame && !findGroupBoundary(groups, frameRange.max)) {
      return "Frame range is not aligned with sprite sheet";
    }
  }
  return {};
}

FrameIdx getMaxFrameCount(const tcb::span<const Group> groups, const FrameRange range) {
  GroupInfo info = findGroup(groups, range.min);
  FrameIdx maxCount = info.end - info.begin;
  FrameIdx prevEnd = info.end;
  std::size_t index = static_cast<std::size_t>(info.group) + 1;
  while (index < groups.size()) {
    const FrameIdx end = groups[index].end;
    if (end - FrameIdx{1} > range.max) break;
    maxCount = std::max(maxCount, end - prevEnd);
    prevEnd = end;
    ++index;
  }
  return std::min(maxCount, range.max - range.min + FrameIdx{1});
}

template <typename Func>
Error eachFrame(const AnimExportParams &params, const Animation &anim, Func func) {
  const tcb::span<const Layer> layers = anim.timeline.getLayerArray();
  LayerRange layerRange = params.layers;
  FrameRange frameRange = params.frames;
  
  TRY(validateRange(layerRange, anim.timeline.getLayers()));
  TRY(validateRange(frameRange, anim.timeline.getFrames()));
  TRY(validateSheetRange(layerRange, frameRange, params.name, anim.timeline));
  
  const LayerIdx layerCount = layerRange.max - layerRange.min + LayerIdx{1};
  
  Frame frame;
  frame.reserve(+layerCount);
  
  std::vector<CelIterator> celIters;
  celIters.reserve(+layerCount);
  for (LayerIdx l = layerRange.min; l <= layerRange.max; ++l) {
    celIters.emplace_back(layers[+l].cels, frameRange.min);
  }
  GroupIterator groupIter{anim.timeline.getGroupArray(), frameRange.min};
  bool changedGroup = true;
  
  SpriteNameState state;
  state.layer = layerRange.min;
  state.layerCount = LayerIdx{1};
  state.groupCount = anim.timeline.getGroups();
  state.frameCount = anim.timeline.getFrames();
  state.maxGroupFrameCount = getMaxFrameCount(anim.timeline.getGroupArray(), frameRange);
  state.layerName = layers[+layerRange.min].name;
  
  for (FrameIdx f = frameRange.min; f <= frameRange.max; ++f) {
    frame.clear();
    for (LayerIdx l = {}; l != layerCount; ++l) {
      if (!includeLayer(layerRange.vis, layers[+(l + layerRange.min)].visible)) continue;
      if (const CelImage *cel = celIters[+l].img(); *cel) {
        frame.push_back(cel);
      }
      celIters[+l].incr();
    }
    
    if (changedGroup) {
      const GroupInfo info = groupIter.info();
      state.group = info.group;
      state.groupFrameCount = info.end - info.begin;
      state.groupBegin = info.begin;
      state.groupName = groupIter.name();
    }
    state.frame = f;
    changedGroup = groupIter.incr();
    
    TRY_VOID(func(frame, state));
  }
  
  return {};
}

template <typename Func>
Error eachCel(const AnimExportParams &params, const Animation &anim, Func func) {
  const tcb::span<const Layer> layers = anim.timeline.getLayerArray();
  LayerRange layerRange = params.layers;
  FrameRange frameRange = params.frames;
  
  TRY(validateRange(layerRange, anim.timeline.getLayers()));
  TRY(validateRange(frameRange, anim.timeline.getFrames()));
  TRY(validateSheetRange(layerRange, frameRange, params.name, anim.timeline));
  
  SpriteNameState state;
  state.layerCount = anim.timeline.getLayers();
  state.groupCount = anim.timeline.getGroups();
  state.frameCount = anim.timeline.getFrames();
  state.maxGroupFrameCount = getMaxFrameCount(anim.timeline.getGroupArray(), frameRange);
  
  for (LayerIdx l = layerRange.min; l <= layerRange.max; ++l) {
    const Layer &layer = layers[+l];
    if (!includeLayer(layerRange.vis, layer.visible)) continue;
    CelIterator celIter{layer.cels, frameRange.min};
    GroupIterator groupIter{anim.timeline.getGroupArray(), frameRange.min};
    bool changedGroup = true;
    
    state.layer = l;
    state.layerName = layer.name;
    
    for (FrameIdx f = frameRange.min; f <= frameRange.max; ++f) {
      if (changedGroup) {
        const GroupInfo info = groupIter.info();
        state.group = info.group;
        state.groupFrameCount = info.end - info.begin;
        state.groupBegin = info.begin;
        state.groupName = groupIter.name();
      }
      
      state.frame = f;
      TRY_VOID(func(celIter.img(), state));
      
      celIter.incr();
      changedGroup = groupIter.incr();
    }
  }
  
  return {};
}

struct Images {
  QImage canvas;
  QImage xformed;
};

bool isIdentity(const SpriteTransform &transform) {
  return transform.scaleX == 1 && transform.scaleY == 1 && transform.angle == 0;
}

QSize getTransformedSize(const QSize canvasSize, const SpriteTransform &transform) {
  QSize size;
  size.setWidth(canvasSize.width() * std::abs(transform.scaleX));
  size.setHeight(canvasSize.height() * std::abs(transform.scaleY));
  return convert(gfx::rotateSize(convert(size), transform.angle));
}

void initImages(Images &images, const AnimExportParams &params, const Animation &anim) {
  Format imageFormat = anim.getFormat();
  if (params.composite && anim.getFormat() != Format::gray) {
    imageFormat = Format::rgba;
  }
  images.canvas = {anim.getSize(), qimageFormat(imageFormat)};
  if (isIdentity(params.transform)) {
    images.xformed = {};
  } else {
    const QSize xformedSize = getTransformedSize(images.canvas.size(), params.transform);
    images.xformed = {xformedSize, images.canvas.format()};
  }
}

void applyTransform(Images &images, const SpriteTransform &transform) {
  assert(transform.scaleX != 0);
  assert(transform.scaleY != 0);
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

const QImage *selectImage(Images &images, const AnimExportParams &animParams) {
  if (images.xformed.isNull()) {
    return &images.canvas;
  } else {
    applyTransform(images, animParams.transform);
    return &images.xformed;
  }
}

void addFrameNames(
  std::size_t &index,
  const ExportParams &params,
  const AnimExportParams &animParams,
  const Animation &anim
) {
  const QSize size = getTransformedSize(anim.getSize(), animParams.transform);
  NameAppender appender{params.generator.get(), animParams.name, size};
  auto iterate = [&](const Frame &frame, const SpriteNameState &state) {
    appender.append(index, state, frame.empty());
  };
  static_cast<void>(eachFrame(animParams, anim, iterate));
}

void addCelNames(
  std::size_t &index,
  const ExportParams &params,
  const AnimExportParams &animParams,
  const Animation &anim
) {
  const QSize size = getTransformedSize(anim.getSize(), animParams.transform);
  NameAppender appender{params.generator.get(), animParams.name, size};
  auto iterate = [&](const CelImage *img, const SpriteNameState &state) {
    appender.append(index, state, img->isNull());
  };
  static_cast<void>(eachCel(animParams, anim, iterate));
}

Error addFrameImages(
  std::size_t &index,
  const ExportParams &params,
  const AnimExportParams &animParams,
  const Animation &anim
) {
  Images images;
  initImages(images, animParams, anim);
  const Format format = anim.getFormat();
  const PaletteCSpan palette = anim.palette.getPalette();
  const QSize size = getTransformedSize(anim.getSize(), animParams.transform);
  ImageCopier copier{params.generator.get(), animParams.name, size, format};
  
  auto iterate = [&](const Frame &frame, const SpriteNameState &state) {
    if (!frame.empty()) {
      if (format == Format::gray) {
        compositeFrame<FmtGray>(images.canvas, palette, frame, format, images.canvas.rect());
      } else {
        compositeFrame<FmtRgba>(images.canvas, palette, frame, format, images.canvas.rect());
      }
      return copier.copy(index, state, selectImage(images, animParams));
    } else {
      return copier.copy(index, state, nullptr);
    }
  };
  
  return eachFrame(animParams, anim, iterate);
}

Error addCelImages(
  std::size_t &index,
  const ExportParams &params,
  const AnimExportParams &animParams,
  const Animation &anim
) {
  Images images;
  initImages(images, animParams, anim);
  const QSize size = getTransformedSize(anim.getSize(), animParams.transform);
  ImageCopier copier{params.generator.get(), animParams.name, size, anim.getFormat()};
  
  auto iterate = [&](const CelImage *cel, const SpriteNameState &state) {
    if (*cel) {
      clearImage(images.canvas);
      blitImage(images.canvas, cel->img, cel->pos);
      return copier.copy(index, state, selectImage(images, animParams));
    } else {
      return copier.copy(index, state, nullptr);
    }
  };
  
  return eachCel(animParams, anim, iterate);
}

Format compositedFormat(const Format format, const bool composite) {
  if (format == Format::index && composite) {
    return Format::rgba;
  } else {
    return format;
  }
}

using AnimPtr = std::unique_ptr<const Animation, void(*)(const Animation *)>;
using AnimArray = std::vector<AnimPtr>;

Error exportTextureAtlas(const ExportParams &params, const AnimArray &anims) {
  assert(params.generator);
  assert(params.anims.size() == anims.size());
  assert(!anims.empty());
  
  for (std::size_t s = 0; s != anims.size(); ++s) {
    const Format format = compositedFormat(anims[s]->getFormat(), params.anims[s].composite);
    if (!params.generator->supported(params.pixelFormat, format)) {
      return "Format is not supported by atlas generator";
    }
  }
  
  AtlasInfo info = {params.name, params.directory, params.pixelFormat};
  if (info.directory.isEmpty()) {
    info.directory = ".";
  }
  TRY(params.generator->beginAtlas(info));
  
  std::size_t spriteIndex = 0;
  for (std::size_t s = 0; s != anims.size(); ++s) {
    if (params.anims[s].composite) {
      addFrameNames(spriteIndex, params, params.anims[s], *anims[s]);
    } else {
      addCelNames(spriteIndex, params, params.anims[s], *anims[s]);
    }
  }
  
  if (params.whitepixel) {
    params.generator->appendWhiteName(spriteIndex);
  }
  
  if (QString name = params.generator->endNames(); !name.isNull()) {
    return "Sprite name collision \"" + name + "\"";
  }
  
  TRY(params.generator->beginImages());
  
  spriteIndex = 0;
  for (std::size_t s = 0; s != anims.size(); ++s) {
    const Format format = compositedFormat(anims[s]->getFormat(), params.anims[s].composite);
    TRY(params.generator->setImageFormat(format, anims[s]->palette.getPalette()));
    if (params.anims[s].composite) {
      TRY(addFrameImages(spriteIndex, params, params.anims[s], *anims[s]));
    } else {
      TRY(addCelImages(spriteIndex, params, params.anims[s], *anims[s]));
    }
  }
  
  if (params.whitepixel) {
    TRY(params.generator->copyWhiteImage(spriteIndex));
  }
  
  return params.generator->endAtlas();
}

}

Error exportTextureAtlas(const ExportParams &params, const std::vector<QString> &paths) {
  // TODO: This is not very efficient
  // We only need to load the selected portion of the file
  // Although the common case is to export the whole thing so
  // we'd be optimizing an uncommon situation
  
  // Also, I have to do all this nonsense instead of using std::vector<Animation>
  // because QObject doesn't have a move constructor. Might be better off using
  // a simpler data structure here. We don't need the full functionality of
  // Animation.
  AnimArray anims;
  for (const QString &path : paths) {
    auto *anim = new Animation;
    if (Error err = anim->openFile(path)) {
      delete anim;
      return err;
    }
    anims.push_back(AnimPtr{anim, [](const Animation *anim) {
      delete anim;
    }});
  }
  
  return exportTextureAtlas(params, anims);
}

Error exportTextureAtlas(const ExportParams &params, const Animation &anim) {
  AnimArray anims;
  anims.push_back(AnimPtr{&anim, [](const Animation *) {}});
  return exportTextureAtlas(params, anims);
}
