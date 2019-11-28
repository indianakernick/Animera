//
//  export options.cpp
//  Animera
//
//  Created by Indi Kernick on 24/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "export options.hpp"

#include "config.hpp"
#include "strings.hpp"
#include <QtCore/qdir.h>
#include "export pattern.hpp"

namespace {

template <typename Idx>
Idx applyLine(const Line<Idx> line, const Idx value) {
  return value * line.stride + line.offset;
}

}

QString getExportPath(const ExportOptions &options, ExportState state) {
  QString path = options.directory;
  if (path.back() != QDir::separator()) {
    path.push_back(QDir::separator());
  }
  state.currPos.l = applyLine(options.layerLine, state.currPos.l);
  state.currPos.f = applyLine(options.frameLine, state.currPos.f);
  path += evalExportPattern(options.name, state);
  path += ".png";
  return path;
}

CellRect getExportRect(const ExportOptions &options, const ExportSpriteInfo &info) {
  CellRect rect;
  switch (options.layerSelect) {
    case LayerSelect::all_composited:
    case LayerSelect::all:
      rect.minL = LayerIdx{0};
      rect.maxL = info.layers - LayerIdx{1};
      break;
    case LayerSelect::current:
      rect.minL = rect.maxL = info.currPos.l;
      break;
    default: Q_UNREACHABLE();
  }
  switch (options.frameSelect) {
    case FrameSelect::all:
      rect.minF = FrameIdx{0};
      rect.maxF = info.frames - FrameIdx{1};
      break;
    case FrameSelect::current:
      rect.minF = rect.maxF = info.currPos.f;
      break;
    default: Q_UNREACHABLE();
  }
  return rect;
}

void initDefaultOptions(ExportOptions &options, const Format canvasFormat) {
  options.name = "sprite_%000F";
  options.directory = ".";
  options.layerLine = {LayerIdx{1}, LayerIdx{0}};
  options.frameLine = {FrameIdx{1}, FrameIdx{0}};
  options.layerSelect = LayerSelect::all_composited;
  options.frameSelect = FrameSelect::all;
  switch (canvasFormat) {
    case Format::rgba:
      options.format = ExportFormat::rgba;
      break;
    case Format::index:
      options.format = ExportFormat::rgba;
      break;
    case Format::gray:
      options.format = ExportFormat::gray_alpha;
      break;
  }
  options.scaleX = 1;
  options.scaleY = 1;
  options.angle = 0;
}

ExportOptions exportFrameOptions(const QString &path, const Format canvasFormat) {
  QString dir = path;
  dir.chop(dir.size() - dir.lastIndexOf('.'));
  const int lastSlash = dir.lastIndexOf(QDir::separator());
  const int nameLen = dir.size() - lastSlash - 1;
  QString name{dir.data() + lastSlash + 1, nameLen};
  dir.chop(nameLen);
  ExportOptions options;
  initDefaultOptions(options, canvasFormat);
  options.name = std::move(name);
  options.directory = std::move(dir);
  return options;
}

namespace {

Error setInt(long &number, const docopt::value &value, const QString &name) {
  try {
    number = value.asLong();
  } catch (std::exception &) {
    return name + " must be an integer";
  }
  return {};
}

QString nonZeroRangeStr(const IntRange range) {
  QString str = "\nValid range is: [";
  str += QString::number(range.min);
  str += ", -1] U [1, ";
  str += QString::number(range.max);
  str += ']';
  return str;
}

QString rangeStr(const IntRange range) {
  QString str = "\nValid range is: [";
  str += QString::number(range.min);
  str += ", ";
  str += QString::number(range.max);
  str += ']';
  return str;
}

Error setInt(
  int &number,
  const docopt::value &value,
  const QString &name,
  const IntRange range
) {
  long longNumber;
  if (Error err = setInt(longNumber, value, name); err) return err;
  if (longNumber < range.min || longNumber > range.max) {
    return name + " is out of range" + rangeStr(range);
  }
  number = static_cast<int>(longNumber);
  return {};
}

Error setNonZeroInt(
  int &number,
  const docopt::value &value,
  const QString &name,
  const IntRange range
) {
  long longNumber;
  if (Error err = setInt(longNumber, value, name); err) return err;
  if (longNumber == 0) {
    return name + " cannot be 0" + nonZeroRangeStr(range);
  }
  if (longNumber < expt_stride.min || longNumber > expt_stride.max) {
    return name + " is out of range" + nonZeroRangeStr(range);
  }
  number = static_cast<int>(longNumber);
  return {};
}

template <typename Idx>
const QString idxName;
template <>
const QString idxName<LayerIdx> = "Layer";
template <>
const QString idxName<FrameIdx> = "Frame";

template <typename Idx>
Error setStride(Line<Idx> &line, const docopt::value &strideValue) {
  const QString name = idxName<Idx> + " stride";
  int stride;
  if (Error err = setNonZeroInt(stride, strideValue, name, expt_stride); err) {
    return err;
  }
  line.stride = Idx{stride};
  return {};
}

template <typename Idx>
Error setOffset(Line<Idx> &line, const docopt::value &offsetValue) {
  const QString name = idxName<Idx> + " offset";
  int offset;
  if (Error err = setInt(offset, offsetValue, name, expt_offset); err) {
    return err;
  }
  line.offset = static_cast<Idx>(offset);
  return {};
}

Error setStrideOffset(ExportOptions &options, const std::map<std::string, docopt::value> &flags) {
  if (const docopt::value &stride = flags.at("--layer-stride"); stride) {
    if (Error err = setStride(options.layerLine, stride); err) return err;
  }
  if (const docopt::value &offset = flags.at("--layer-offset"); offset) {
    if (Error err = setOffset(options.layerLine, offset); err) return err;
  }
  if (const docopt::value &stride = flags.at("--frame-stride"); stride) {
    if (Error err = setStride(options.frameLine, stride); err) return err;
  }
  if (const docopt::value &offset = flags.at("--frame-offset"); offset) {
    if (Error err = setOffset(options.frameLine, offset); err) return err;
  }
  return {};
}

Error setScale(ExportOptions &options, const std::map<std::string, docopt::value> &flags) {
  bool scaleXorY = false;
  if (const docopt::value &scaleX = flags.at("--scale-x"); scaleX) {
    if (Error err = setNonZeroInt(options.scaleX, scaleX, "scale-x", expt_scale); err) {
      return err;
    }
    scaleXorY = true;
  }
  if (const docopt::value &scaleY = flags.at("--scale-y"); scaleY) {
    if (Error err = setNonZeroInt(options.scaleY, scaleY, "scale-y", expt_scale); err) {
      return err;
    }
    scaleXorY = true;
  }
  if (const docopt::value &scale = flags.at("--scale"); scale) {
    if (scaleXorY) {
      // The error message from docopt is useless
      return "scale-x and scale-y are mutually exclusive with scale";
    }
    int scaleXY;
    if (Error err = setNonZeroInt(scaleXY, scale, "scale", expt_scale); err) {
      return err;
    }
    options.scaleX = options.scaleY = scaleXY;
  }
  return {};
}

}

Error readExportOptions(
  ExportOptions &options,
  CellPos &current,
  const Format format,
  const std::map<std::string, docopt::value> &flags
) {
  if (const docopt::value &name = flags.at("--name"); name) {
    options.name = toLatinString(name.asString());
  }
  
  if (const docopt::value &dir = flags.at("--directory"); dir) {
    options.directory = toLatinString(dir.asString());
  }
  
  if (Error err = setStrideOffset(options, flags); err) return err;
  
  const bool composite = !flags.at("--no-composite").asBool();
  
  if (Error err = setScale(options, flags); err) return err;
  
  if (const docopt::value &angle = flags.at("--angle"); angle) {
    long angleLong;
    if (Error err = setInt(angleLong, angle, "angle"); err) return err;
    angleLong &= 3;
    options.angle = static_cast<int>(angleLong);
  }
  
  return {};
}
