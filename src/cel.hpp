//
//  cel.hpp
//  Animera
//
//  Created by Indiana Kernick on 17/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_cel_hpp
#define animera_cel_hpp

#include <memory>
#include <vector>
#include "image.hpp"
#include "enum operators.hpp"

class CelImage;

// TODO: Can we merge CelImage and Cel into one class?
// do we really need stable cel image pointers?
using CelImagePtr = std::unique_ptr<CelImage>;

class CelImage {
public:
  QImage img;
  QPoint pos;
  
  explicit operator bool() const {
    return !img.isNull();
  }
  bool isNull() const {
    return img.isNull();
  }
  QRect rect() const {
    return {pos, img.size()};
  }
};

enum class LayerIdx {};
enum class GroupIdx {};
enum class FrameIdx {};

template <>
struct enum_unary_plus<LayerIdx> : std::true_type {};

template <>
struct enum_unary_plus<GroupIdx> : std::true_type {};

template <>
struct enum_unary_plus<FrameIdx> : std::true_type {};

template <>
struct enum_math<LayerIdx> : std::true_type {};

template <>
struct enum_math<GroupIdx> : std::true_type {};

template <>
struct enum_math<FrameIdx> : std::true_type {};

struct CelPos {
  LayerIdx l;
  FrameIdx f;
};

struct CelRect {
  LayerIdx minL;
  FrameIdx minF;
  LayerIdx maxL;
  FrameIdx maxF;
};

constexpr CelRect empty_rect = {LayerIdx{0}, FrameIdx{0}, LayerIdx{-1}, FrameIdx{-1}};

using Frame = std::vector<const CelImage *>;

struct GroupInfo {
  GroupIdx group;
  FrameIdx begin;
  FrameIdx end;
};

#endif
