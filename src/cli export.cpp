//
//  cli export.cpp
//  Animera
//
//  Created by Indiana Kernick on 11/4/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "cli export.hpp"

#include <charconv>
#include "sprite.hpp"
#include "strings.hpp"
#include <QtCore/qdir.h>
#include "docopt helpers.hpp"
#include "export pattern.hpp"
#include "export options.hpp"
#include <QtCore/qtextstream.h>
#include <QtCore/qcoreapplication.h>

namespace {

template <typename Idx>
const QString idxName;
template <>
const QString idxName<LayerIdx> = "Layer";
template <>
const QString idxName<FrameIdx> = "Frame";

const char *formatNames[] = {
  "rgba",
  "index",
  "gray",
  "gray-alpha",
  "monochrome"
};

QString formatNamesList(std::initializer_list<ExportFormat> formats) {
  return validListStr("formats", formats.size(), [formats](QString &str, std::size_t i) {
    str += formatNames[static_cast<std::size_t>(formats.begin()[i])];
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
  CelRect &selection,
  const ExportSpriteInfo info,
  const docopt::value &value
) {
  IntRange range;
  TRY(setRange(range, info.layers, value));
  selection.minL = LayerIdx{range.min};
  selection.maxL = LayerIdx{range.max};
  return {};
}

Error setFrame(
  CelRect &selection,
  const ExportSpriteInfo info,
  const docopt::value &value
) {
  IntRange range;
  TRY(setRange(range, info.frames, value));
  selection.minF = FrameIdx{range.min};
  selection.maxF = FrameIdx{range.max};
  return {};
}

Error setNameDir(ExportOptions &options, const docopt::Options &flags) {
  if (const docopt::value &name = flags.at("--name"); name) {
    QString nameStr = toLatinString(name.asString());
    TRY(checkExportPattern(nameStr));
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

Error setLayerFrame(
  ExportOptions &options,
  const ExportSpriteInfo info,
  const docopt::Options &flags
) {
  if (const docopt::value &layer = flags.at("--layer"); layer) {
    TRY(setLayer(options.selection, info, layer));
  }
  if (const docopt::value &frame = flags.at("--frame"); frame) {
    TRY(setFrame(options.selection, info, frame));
  }
  return {};
}

Error setFormat(
  ExportOptions &options,
  const ExportSpriteInfo info,
  const docopt::Options &flags
) {
  if (const docopt::value &value = flags.at("--format"); value) {
    TRY(setFormat(options.format, value));
    TRY(checkFormat(options, info.format));
  }
  return {};
}

Error setVisibility(ExportOptions &options, const docopt::Options &flags) {
  if (const docopt::value &value = flags.at("--visibility"); value) {
    TRY(setVisibility(options.visibility, value));
  }
  return {};
}

Error setScaleAngle(ExportOptions &options, const docopt::Options &flags) {
  if (const docopt::value &scaleX = flags.at("--scale-x"); scaleX) {
    TRY(setNonZeroInt(options.scaleX, scaleX, "scale-x", expt_scale));
  }
  if (const docopt::value &scaleY = flags.at("--scale-y"); scaleY) {
    TRY(setNonZeroInt(options.scaleY, scaleY, "scale-y", expt_scale));
  }
  if (const docopt::value &scale = flags.at("--scale"); scale) {
    int scaleXY;
    TRY(setNonZeroInt(scaleXY, scale, "scale", expt_scale));
    options.scaleX = options.scaleY = scaleXY;
  }
  if (const docopt::value &angle = flags.at("--angle"); angle) {
    long angleLong;
    TRY(setInt(angleLong, angle, "angle"));
    angleLong &= 3;
    options.angle = static_cast<int>(angleLong);
  }
  return {};
}

Error readExportOptions(
  ExportOptions &options,
  const ExportSpriteInfo &info,
  const docopt::Options &flags
) {
  TRY(setNameDir(options, flags));
  TRY(setLayerFrame(options, info, flags));
  options.composite = !flags.at("--no-composite").asBool();
  TRY(setFormat(options, info, flags));
  TRY(setVisibility(options, flags));
  TRY(setScaleAngle(options, flags));
  return {};
}

}

int cliExport(int &argc, char **argv, const docopt::Options &flags) {
  QCoreApplication app{argc, argv};
  QTextStream console{stdout};
  
  Sprite sprite;
  if (Error err = sprite.openFile(toLatinString(flags.at("<file>").asString())); err) {
    console << "File open error\n";
    console << err.msg() << '\n';
    return 1;
  }
  
  ExportOptions options;
  const ExportSpriteInfo info = getSpriteInfo(sprite);
  initDefaultOptions(options, info);
  if (Error err = readExportOptions(options, info, flags); err) {
    console << "Configuration error\n";
    console << err.msg() << '\n';
    return 1;
  }
  
  if (Error err = sprite.exportSprite(options); err) {
    console << "Export error\n";
    console << err.msg() << '\n';
    return 1;
  }
  
  return 0;
}
