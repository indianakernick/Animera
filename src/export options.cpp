//
//  export options.cpp
//  Animera
//
//  Created by Indi Kernick on 24/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "export options.hpp"

#include <charconv>
#include "sprite.hpp"
#include "config.hpp"
#include "strings.hpp"
#include <QtCore/qdir.h>
#include "docopt helpers.hpp"
#include "export pattern.hpp"

ExportSpriteInfo getSpriteInfo(const Sprite &sprite) {
  return {sprite.timeline.getLayers(), sprite.timeline.getFrames(), sprite.getFormat()};
}

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

void initDefaultOptions(ExportOptions &options, const ExportSpriteInfo info) {
  options.name = "sprite_%000F";
  options.directory = ".";
  options.layerLine = {LayerIdx{1}, LayerIdx{0}};
  options.frameLine = {FrameIdx{1}, FrameIdx{0}};
  options.selection.minL = LayerIdx{};
  options.selection.minF = FrameIdx{};
  options.selection.maxL = info.layers - LayerIdx{1};
  options.selection.maxF = info.frames - FrameIdx{1};
  switch (info.format) {
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
  options.visibility = ExportVis::visible;
  options.scaleX = 1;
  options.scaleY = 1;
  options.angle = 0;
  options.composite = true;
}

ExportOptions exportFrameOptions(const QString &path, const ExportSpriteInfo info) {
  QString dir = path;
  dir.chop(dir.size() - dir.lastIndexOf('.'));
  const int lastSlash = dir.lastIndexOf(QDir::separator());
  const int nameLen = dir.size() - lastSlash - 1;
  QString name{dir.data() + lastSlash + 1, nameLen};
  dir.chop(nameLen);
  ExportOptions options;
  initDefaultOptions(options, info);
  options.name = std::move(name);
  options.directory = std::move(dir);
  return options;
}

namespace {

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
  return validListStr("formats", formats.size(), [formats](QString &str, size_t i) {
    str += formatNames[static_cast<size_t>(formats.begin()[i])];
  });
}

Error setFormat(ExportFormat &format, const docopt::value &formatValue) {
  if (!setEnum(format, formatValue.asString(), formatNames)) {
    return "Invalid export format" + validListStr("formats", formatNames);
  }
  return {};
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

Error checkFormat(const ExportOptions &options, const Format canvasFormat) {
  switch (canvasFormat) {
    case Format::rgba:
      return checkFormat(options.format, "rgba", {ExportFormat::rgba});
    case Format::index:
      if (options.composite) {
        return checkFormat(options.format, "index", {ExportFormat::rgba});
      } else {
        return checkFormat(options.format, "index", {
          ExportFormat::index, ExportFormat::gray, ExportFormat::monochrome
        });
      }
    case Format::gray:
      return checkFormat(options.format, "gray", {
        ExportFormat::gray_alpha, ExportFormat::gray, ExportFormat::monochrome
      });
  }
  return {};
}

const char *visNames[] = {
  "visible",
  "hidden",
  "all"
};

Error setVisibility(ExportVis &visibility, const docopt::value &visValue) {
  if (!setEnum(visibility, visValue.asString(), visNames)) {
    return "Invalid visibility mode" + validListStr("modes", visNames);
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
  CellRect &selection,
  const ExportSpriteInfo info,
  const docopt::value &value
) {
  IntRange range;
  if (Error err = setRange(range, info.layers, value); err) {
    return err;
  }
  selection.minL = LayerIdx{range.min};
  selection.maxL = LayerIdx{range.max};
  return {};
}

Error setFrame(
  CellRect &selection,
  const ExportSpriteInfo info,
  const docopt::value &value
) {
  IntRange range;
  if (Error err = setRange(range, info.frames, value); err) {
    return err;
  }
  selection.minF = FrameIdx{range.min};
  selection.maxF = FrameIdx{range.max};
  return {};
}

Error setNameDir(ExportOptions &options, const std::map<std::string, docopt::value> &flags) {
  if (const docopt::value &name = flags.at("--name"); name) {
    QString nameStr = toLatinString(name.asString());
    if (Error err = checkExportPattern(nameStr); err) {
      return err;
    }
    options.name = std::move(nameStr);
  }
  if (const docopt::value &dir = flags.at("--directory"); dir) {
    QString dirStr = toLatinString(dir.asString());
    if (dirStr.isEmpty()) {
      return "Directory must not be empty";
    }
    if (!QDir{dirStr}.exists()) {
      return "Invalid directory";
    }
    options.directory = std::move(dirStr);
  }
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

Error setLayerFrame(
  ExportOptions &options,
  const ExportSpriteInfo info,
  const std::map<std::string, docopt::value> &flags
) {
  if (const docopt::value &layer = flags.at("--layer"); layer) {
    if (Error err = setLayer(options.selection, info, layer); err) {
      return err;
    }
  }
  if (const docopt::value &frame = flags.at("--frame"); frame) {
    if (Error err = setFrame(options.selection, info, frame); err) {
      return err;
    }
  }
  return {};
}

Error setFormat(
  ExportOptions &options,
  const ExportSpriteInfo info,
  const std::map<std::string, docopt::value> &flags
) {
  if (const docopt::value &value = flags.at("--format"); value) {
    if (Error err = setFormat(options.format, value); err) {
      return err;
    }
    if (Error err = checkFormat(options, info.format); err) {
      return err;
    }
  }
  return {};
}

Error setVisibility(ExportOptions &options, const std::map<std::string, docopt::value> &flags) {
  if (const docopt::value &value = flags.at("--visibility"); value) {
    if (Error err = setVisibility(options.visibility, value); err) {
      return err;
    }
  }
  return {};
}

Error setScaleAngle(ExportOptions &options, const std::map<std::string, docopt::value> &flags) {
  if (const docopt::value &scaleX = flags.at("--scale-x"); scaleX) {
    if (Error err = setNonZeroInt(options.scaleX, scaleX, "scale-x", expt_scale); err) {
      return err;
    }
  }
  if (const docopt::value &scaleY = flags.at("--scale-y"); scaleY) {
    if (Error err = setNonZeroInt(options.scaleY, scaleY, "scale-y", expt_scale); err) {
      return err;
    }
  }
  if (const docopt::value &scale = flags.at("--scale"); scale) {
    int scaleXY;
    if (Error err = setNonZeroInt(scaleXY, scale, "scale", expt_scale); err) {
      return err;
    }
    options.scaleX = options.scaleY = scaleXY;
  }
  if (const docopt::value &angle = flags.at("--angle"); angle) {
    long angleLong;
    if (Error err = setInt(angleLong, angle, "angle"); err) return err;
    angleLong &= 3;
    options.angle = static_cast<int>(angleLong);
  }
  return {};
}

}

Error readExportOptions(
  ExportOptions &options,
  const ExportSpriteInfo info,
  const std::map<std::string, docopt::value> &flags
) {
  if (Error err = setNameDir(options, flags); err) return err;
  if (Error err = setStrideOffset(options, flags); err) return err;
  if (Error err = setLayerFrame(options, info, flags); err) return err;
  options.composite = !flags.at("--no-composite").asBool();
  if (Error err = setFormat(options, info, flags); err) return err;
  if (Error err = setVisibility(options, flags); err) return err;
  if (Error err = setScaleAngle(options, flags); err) return err;
  return {};
}
