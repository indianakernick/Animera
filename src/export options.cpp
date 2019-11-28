//
//  export options.cpp
//  Animera
//
//  Created by Indi Kernick on 24/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "export options.hpp"

#include <charconv>
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
      rect.minL = rect.maxL = info.current.l;
      break;
    default: Q_UNREACHABLE();
  }
  switch (options.frameSelect) {
    case FrameSelect::all:
      rect.minF = FrameIdx{0};
      rect.maxF = info.frames - FrameIdx{1};
      break;
    case FrameSelect::current:
      rect.minF = rect.maxF = info.current.f;
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

const char *formatNames[] = {
  "rgba",
  "index",
  "gray",
  "gray-alpha",
  "monochrome"
};

QString formatNamesList(std::initializer_list<ExportFormat> formats) {
  QString str = "\nValid formats are: {";
  bool first = true;
  for (ExportFormat format : formats) {
    if (!std::exchange(first, false)) str += ", ";
    str += formatNames[static_cast<size_t>(format)];
  }
  str += '}';
  return str;
}

QString formatNamesList() {
  return formatNamesList({
    ExportFormat::rgba,
    ExportFormat::index,
    ExportFormat::gray,
    ExportFormat::gray_alpha,
    ExportFormat::monochrome
  });
}

void toLowerStr(std::string &str) {
  std::transform(str.begin(), str.end(), str.begin(), [](unsigned char ch) {
    return std::tolower(ch);
  });
}

Error setExportFormat(ExportFormat &format, const docopt::value &formatValue) {
  assert(formatValue.isString());
  std::string formatStr = formatValue.asString();
  toLowerStr(formatStr);
  for (size_t i = 0; i != std::size(formatNames); ++i) {
    if (formatStr == formatNames[i]) {
      format = static_cast<ExportFormat>(i);
      return {};
    }
  }
  return "Invalid export format" + formatNamesList();
}

Error checkFormat(
  const ExportFormat format,
  const QString &canvasFormat,
  std::initializer_list<ExportFormat> formats
) {
  if (std::find(formats.begin(), formats.end(), format) == formats.end()) {
    QString msg = "Invalid export format for ";
    msg += canvasFormat;
    msg += " sprite format";
    msg += formatNamesList(formats);
    return msg;
  }
  return {};
}

Error checkFormat(
  const ExportFormat format,
  const Format canvasFormat,
  const bool composite
) {
  switch (canvasFormat) {
    case Format::rgba:
      return checkFormat(format, "rgba", {ExportFormat::rgba});
    case Format::index:
      if (composite) {
        return checkFormat(format, "index", {ExportFormat::rgba});
      } else {
        return checkFormat(format, "index", {ExportFormat::index, ExportFormat::gray, ExportFormat::monochrome});
      }
    case Format::gray:
      return checkFormat(format, "gray", {ExportFormat::gray_alpha, ExportFormat::gray, ExportFormat::monochrome});
  }
  return {};
}

const QString rangeFormats = "\nValid range formats are: {n, n..n, ..n, n.., ..}";

template <typename Idx>
Error setRange(IntRange &range, const Idx length, const docopt::value &value) {
  // TODO: should we use long here?
  // For consistency of error messages
  // if we use long
  //   3'000'000'000 is out of range
  // if we use int
  //   3'000'000'000 is not an integer
  assert(value.isString());
  const std::string &str = value.asString();
  if (str.empty()) {
    return idxName<Idx> + " range is empty" + rangeFormats;
  }
  const char *pos = str.data();
  const char *const strEnd = pos + str.size();
  if (pos[0] != '.') {
    const auto [end, err] = std::from_chars(pos, strEnd, range.min);
    if (err != std::errc{}) {
      return idxName<Idx> + " range min must be an integer" + rangeFormats;
    }
    pos = end;
    if (pos == strEnd) {
      range.max = range.min;
      if (range.min < 0 || range.min >= +length) {
        return idxName<Idx> + " is out of range" + rangeStr({0, +length - 1});
      }
      return {};
    }
  } else {
    range.min = 0;
  }
  if (pos[0] != '.' || pos + 1 == strEnd || pos[1] != '.') {
    return idxName<Idx> + " range separator must be .." + rangeFormats;
  }
  pos += 2;
  if (pos == strEnd) {
    range.max = +length - 1;
  } else {
    const auto [end, err] = std::from_chars(pos, strEnd, range.max);
    pos = end;
    if (err != std::errc{} || pos != strEnd) {
      return idxName<Idx> + " range max must be an integer" + rangeFormats;
    }
  }
  if (range.min > range.max) {
    return idxName<Idx> + " range min must be <= max";
  }
  if (range.min < 0 || range.min >= +length) {
    return idxName<Idx> + " range min is out of range" + rangeStr({0, +length - 1});
  }
  if (range.max < 0 || range.max >= +length) {
    return idxName<Idx> + " range max is out of range" + rangeStr({0, +length - 1});
  }
  return {};
}

Error setLayer(
  LayerSelect &select,
  ExportSpriteInfo &info,
  const docopt::value &value,
  const bool composite
) {
  IntRange range;
  if (Error err = setRange(range, info.layers, value); err) {
    return err;
  }
  if (range.min == range.max) {
    select = LayerSelect::current;
    info.current.l = LayerIdx{range.min};
    return {};
  }
  if (range.min == 0 && range.max == +info.layers - 1) {
    select = composite ? LayerSelect::all_composited : LayerSelect::all;
    return {};
  }
  // select = LayerSelect::selection;
  // info.selection.minL = LayerIdx{range.min};
  // info.selection.maxL = LayerIdx{range.max};
  return "I haven't implemented that yet";
}

Error setFrame(
  FrameSelect &select,
  ExportSpriteInfo &info,
  const docopt::value &value
) {
  IntRange range;
  if (Error err = setRange(range, info.frames, value); err) {
    return err;
  }
  if (range.min == range.max) {
    select = FrameSelect::current;
    info.current.f = FrameIdx{range.min};
    return {};
  }
  if (range.min == 0 && range.max == +info.frames - 1) {
    select = FrameSelect::all;
    return {};
  }
  // select = FrameSelect::selection;
  // info.selection.minF = FrameIdx{range.min};
  // info.selection.maxF = FrameIdx{range.max};
  return "I haven't implemented that yet";
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
  ExportSpriteInfo &info,
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
  
  if (const docopt::value &layer = flags.at("--layer"); layer) {
    if (Error err = setLayer(options.layerSelect, info, layer, composite); err) {
      return err;
    }
  }
  if (const docopt::value &frame = flags.at("--frame"); frame) {
    if (Error err = setFrame(options.frameSelect, info, frame); err) {
      return err;
    }
  }
  
  if (const docopt::value &value = flags.at("--format"); value) {
    if (Error err = setExportFormat(options.format, value); err) {
      return err;
    }
    if (Error err = checkFormat(options.format, format, composite); err) {
      return err;
    }
  }
  
  if (Error err = setScale(options, flags); err) return err;
  
  if (const docopt::value &angle = flags.at("--angle"); angle) {
    long angleLong;
    if (Error err = setInt(angleLong, angle, "angle"); err) return err;
    angleLong &= 3;
    options.angle = static_cast<int>(angleLong);
  }
  
  return {};
}
