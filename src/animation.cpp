//
//  animation.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 17/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "animation.hpp"

#include "serial.hpp"

namespace {

CellPtr cloneOrNull(const CellPtr &cell) {
  return cell ? cell->clone() : nullptr;
}

void insertNull(Frames &frames, const Frames::iterator begin, const size_t count) {
  // Iterator invalidation
  frames.resize(frames.size() + count);
  std::rotate(begin, frames.end() - count, frames.end());
}

}

void Animation::setPalette(Palette *newPalette) {
  if (format == Format::palette) {
    palette = newPalette;
  }
}

bool Animation::hasLayer(const LayerIdx l) const {
  return l < layers.size();
}

bool Animation::hasFrame(const CellPos pos) const {
  return hasLayer(pos.l) && pos.f < layers[pos.l].size();
}

bool Animation::hasFrame(const LayerIdx l, const FrameIdx f) const {
  return hasFrame({l, f});
}

LayerIdx Animation::layerCount() const {
  return static_cast<LayerIdx>(layers.size());
}

FrameIdx Animation::frameCount(const LayerIdx l) const {
  return hasLayer(l) ? static_cast<FrameIdx>(layers[l].size()) : 0;
}

Cell *Animation::getCell(const CellPos pos) const {
  if (pos.l < layers.size()) {
    const Frames &frames = layers[pos.l];
    if (pos.f < frames.size()) {
      return frames[pos.f].get();
    }
  }
  return nullptr;
}

Cell *Animation::getCell(const LayerIdx l, const FrameIdx f) const {
  return getCell({l, f});
}

Frame Animation::getFrame(const FrameIdx f) const {
  Frame frame;
  frame.reserve(layers.size());
  for (LayerIdx l = 0; l != layers.size(); ++l) {
    frame.push_back(getCell(l, f));
  }
  return frame;
}

const Palette &Animation::getPallete() const {
  return *palette;
}

Layers Animation::copyRect(const CellRect rect) const {
  assert(validRect(rect));
  Layers copy;
  copy.resize(rect.maxL - rect.minL + 1);
  for (LayerIdx l = rect.minL; l <= rect.maxL; ++l) {
    const Frames &frames = layers[l];
    if (rect.minF >= frames.size()) continue;
    const FrameIdx endFrame = std::min(rect.maxF + 1, static_cast<FrameIdx>(frames.size()));
    for (FrameIdx f = rect.minF; f < endFrame; ++f) {
      copy[l - rect.minL].push_back(cloneOrNull(frames[f]));
    }
  }
  return copy;
}

void Animation::removeRect(const CellRect rect) {
  assert(validRect(rect));
  for (LayerIdx l = rect.minL; l <= rect.maxL; ++l) {
    Frames &frames = layers[l];
    if (rect.minF >= frames.size()) continue;
    const FrameIdx endFrame = std::min(rect.maxF + 1, static_cast<FrameIdx>(frames.size()));
    frames.erase(frames.begin() + rect.minF, frames.begin() + endFrame);
    updateLayer(l);
  }
}

void Animation::clearRect(const CellRect rect) {
  assert(validRect(rect));
  for (LayerIdx l = rect.minL; l <= rect.maxL; ++l) {
    Frames &frames = layers[l];
    if (rect.minF >= frames.size()) continue;
    const FrameIdx endFrame = std::min(rect.maxF + 1, static_cast<FrameIdx>(frames.size()));
    std::fill(frames.begin() + rect.minF, frames.begin() + endFrame, nullptr);
    updateLayer(l);
  }
}

void Animation::pasteRect(const CellRect rect, const Layers &src) {
  assert(validRect(rect));
  const size_t endLayer = std::min(rect.minL + src.size(), layers.size());
  for (LayerIdx l = rect.minL; l != endLayer; ++l) {
    Frames &frames = layers[l];
    const Frames &srcFrames = src[l - rect.minL];
    if (rect.minF > frames.size()) {
      const size_t leadingNull = rect.minF - frames.size() + 1;
      insertNull(frames, frames.end(), srcFrames.size() + leadingNull);
    } else {
      insertNull(frames, frames.begin() + rect.minF, srcFrames.size());
    }
    for (FrameIdx f = 0; f != srcFrames.size(); ++f) {
      frames[rect.minF + f] = cloneOrNull(srcFrames[f]);
    }
    updateLayer(l);
  }
}

void Animation::appendLayer() {
  layers.emplace_back();
}

const Cell *Animation::getLastCell(const LayerIdx l) const {
  assert(hasLayer(l));
  return layers[l].empty() ? nullptr : layers[l].back().get();
}

void Animation::removeTrailingNull(const LayerIdx l) {
  assert(hasLayer(l));
  if (layers[l].empty()) return;
  Frames &frames = layers[l];
  while (frames.back() == nullptr) {
    frames.pop_back();
  }
}

void Animation::updateLayer(const LayerIdx l) {
  removeTrailingNull(l);
}

bool Animation::validRect(const CellRect rect) const {
  return
    rect.minL < rect.maxL &&
    rect.minF < rect.maxF &&
    hasLayer(rect.minL) &&
    hasLayer(rect.maxL);
}
