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
Error eachFrame(const AnimExportParams &params, const Animation &anim, Func func) {
  const tcb::span<const Layer> layers = anim.timeline.getLayerArray();
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
  GroupIterator groupIter{anim.timeline.getGroupArray(), frameRange.min};
  bool changedGroup = true;
  
  ExportNameState state;
  state.layer = layerRange.min;
  state.layerCount = LayerIdx{1};
  state.groupCount = anim.timeline.getGroups();
  state.layerName = layers[+layerRange.min].name;
  
  for (FrameIdx f = frameRange.min; f <= frameRange.max; ++f) {
    frame.clear();
    for (LayerIdx l = {}; l != layerCount; ++l) {
      if (!includeLayer(layerRange.vis, layers[+(l + layerRange.min)].visible)) continue;
      if (const Cel *cel = *celIters[+l]; *cel) {
        frame.push_back(cel);
      }
      ++celIters[+l];
    }
    
    if (changedGroup) {
      const GroupInfo info = groupIter.info();
      state.group = info.group;
      state.frameCount = info.end - info.begin;
      state.groupBegin = info.begin;
      state.groupName = groupIter.name();
    }
    state.frame = f - state.groupBegin;
    changedGroup = groupIter.incr();
    
    TRY(func(frame, state));
  }
  
  return {};
}

template <typename Func>
Error eachCel(const AnimExportParams &params, const Animation &anim, Func func) {
  const tcb::span<const Layer> layers = anim.timeline.getLayerArray();
  const LayerRange layerRange = params.layers;
  const FrameRange frameRange = params.frames;
  
  ExportNameState state;
  state.layerCount = anim.timeline.getLayers();
  state.groupCount = anim.timeline.getGroups();
  
  for (LayerIdx l = layerRange.min; l <= layerRange.max; ++l) {
    const Layer &layer = layers[+l];
    if (!includeLayer(layerRange.vis, layer.visible)) continue;
    auto celIter = layer.spans.find(frameRange.min);
    GroupIterator groupIter{anim.timeline.getGroupArray(), frameRange.min};
    bool changedGroup = true;
    
    state.layer = l;
    state.layerName = layer.name;
    
    for (FrameIdx f = frameRange.min; f <= frameRange.max; ++f) {
      if (changedGroup) {
        const GroupInfo info = groupIter.info();
        state.group = info.group;
        state.frameCount = info.end - info.begin;
        state.groupBegin = info.begin;
        state.groupName = groupIter.name();
      }
      
      if (const Cel *cel = *celIter; *cel) {
        state.frame = f - state.groupBegin;
        TRY(func(cel, state));
      }
      
      ++celIter;
      changedGroup = groupIter.incr();
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

void initImages(Images &images, const AnimExportParams &params, const Animation &anim) {
  Format imageFormat = anim.getFormat();
  if (params.composite && anim.getFormat() != Format::gray) {
    imageFormat = Format::rgba;
  }
  images.canvas = {anim.getSize(), qimageFormat(imageFormat)};
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
  const AnimExportParams &animParams,
  Images &images
) {
  if (images.xformed.isNull()) {
    return params.backend->addImage(index, images.canvas);
  } else {
    applyTransform(images, animParams.transform);
    return params.backend->addImage(index, images.xformed);
  }
}

void addFrameNames(
  std::size_t &index,
  const ExportParams &params,
  const AnimExportParams &animParams,
  const Animation &anim
) {
  auto iterate = [&](const Frame &, const ExportNameState &state) {
    params.backend->addName(index++, animParams.name, state);
    return Error{};
  };
  static_cast<void>(eachFrame(animParams, anim, iterate));
}

void addCelNames(
  std::size_t &index,
  const ExportParams &params,
  const AnimExportParams &animParams,
  const Animation &anim
) {
  auto iterate = [&](const Cel *, const ExportNameState &state) {
    params.backend->addName(index++, animParams.name, state);
    return Error{};
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
  
  auto iterate = [&](const Frame &frame, const ExportNameState &) {
    if (format == Format::gray) {
      compositeFrame<FmtGray>(images.canvas, palette, frame, format, images.canvas.rect());
    } else {
      compositeFrame<FmtRgba>(images.canvas, palette, frame, format, images.canvas.rect());
    }
    return addImage(index++, params, animParams, images);
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
  
  auto iterate = [&](const Cel *cel, const ExportNameState &) {
    clearImage(images.canvas);
    blitImage(images.canvas, cel->img, cel->pos);
    return addImage(index++, params, animParams, images);
  };
  
  return eachCel(animParams, anim, iterate);
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

using AnimPtr = std::unique_ptr<const Animation, void(*)(const Animation *)>;
using AnimArray = std::vector<AnimPtr>;

Error exportTextureAtlas(const ExportParams &params, const AnimArray &anims) {
  assert(params.backend);
  assert(params.anims.size() == anims.size());
  assert(!anims.empty());
  
  for (std::size_t s = 0; s != anims.size(); ++s) {
    if (!compatibleFormat(anims[s]->getFormat(), params.anims[s].composite, params.pixelFormat)) {
      return "Pixel format is not compatible with animation format";
    }
  }
  
  TRY(params.backend->initAtlas(params.pixelFormat, params.name, params.directory));
  
  if (params.whitepixel) {
    params.backend->addWhiteName();
  }
  
  std::size_t spriteIndex = 0;
  for (std::size_t s = 0; s != anims.size(); ++s) {
    std::size_t indexBefore = spriteIndex;
    if (params.anims[s].composite) {
      addFrameNames(spriteIndex, params, params.anims[s], *anims[s]);
    } else {
      addCelNames(spriteIndex, params, params.anims[s], *anims[s]);
    }
    params.backend->addSizes(
      spriteIndex - indexBefore,
      getTransformedSize(anims[s]->getSize(), params.anims[s].transform)
    );
  }
  
  if (QString name = params.backend->hasNameCollision(); !name.isNull()) {
    return "Sprite name collision \"" + name + "\"";
  }
  
  if (params.whitepixel) {
    TRY(params.backend->addWhiteImage());
  }
  
  spriteIndex = 0;
  for (std::size_t s = 0; s != anims.size(); ++s) {
    TRY(params.backend->initAnimation(
      anims[s]->getFormat(),
      anims[s]->palette.getPalette()
    ));
    if (params.anims[s].composite) {
      TRY(addFrameImages(spriteIndex, params, params.anims[s], *anims[s]));
    } else {
      TRY(addCelImages(spriteIndex, params, params.anims[s], *anims[s]));
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
