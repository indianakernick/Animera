//
//  abstract export params.hpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_abstract_export_params_hpp
#define animera_abstract_export_params_hpp

#include "group array.hpp"
#include "export params.hpp"

// Can be used in place of their low level counter parts

enum class LayerSelection {
  // export dialog.cpp depends on order
  visible,
  hidden,
  all,
  current,
  selected
};

enum class FrameSelection {
  // export dialog.cpp depends on order
  all,
  current,
  selected,
};

struct ExportAnimationInfo {
  CelPos pos;
  CelRect selection;
  LayerIdx layerCount;
  FrameIdx frameCount;
  Format format;
};

class Animation;

ExportAnimationInfo getAnimationInfo(const Animation &);

LayerRange selectLayers(const ExportAnimationInfo &, LayerSelection);
FrameRange selectFrames(const ExportAnimationInfo &, FrameSelection);
FrameRange selectFrames(tcb::span<const Group>, GroupIdx);

ExportParams exportFrameParams(const ExportAnimationInfo &, const QString &);
ExportParams exportCelParams(const ExportAnimationInfo &, const QString &);

#endif
