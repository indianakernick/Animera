//
//  export params.hpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_export_params_hpp
#define animera_export_params_hpp

#include "cel.hpp"
#include "export name.hpp"
#include "palette span.hpp"
#include "atlas generator.hpp"

enum class LayerVis {
  // export cli.cpp depends on order
  visible,
  hidden,
  all
};

struct LayerRange {
  LayerIdx min;
  LayerIdx max;
  LayerVis vis;
};

struct FrameRange {
  FrameIdx min;
  FrameIdx max;
};

struct ExportTransform {
  int scaleX;
  int scaleY;
  int angle;
};

struct AnimExportParams {
  ExportNameParams name;
  ExportTransform transform;
  LayerRange layers;
  FrameRange frames;
  bool composite;
};

struct ExportParams {
  QString name;
  QString directory;
  PixelFormat pixelFormat;
  std::unique_ptr<AtlasGenerator> generator;
  std::vector<AnimExportParams> anims;
  bool whitepixel;
};

#endif
