//
//  cli export.cpp
//  Animera
//
//  Created by Indiana Kernick on 11/4/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "cli export.hpp"

#include <iostream>
#include <QtCore/qdir.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qjsondocument.h>
#include "png atlas generator.hpp"
#include "cpp atlas generator.hpp"
#include "json atlas generator.hpp"
#include "export texture atlas.hpp"
#include <QtCore/qcoreapplication.h>

namespace {

// ---------------------------- strings -----------------------------

template <typename Enum>
const int enumStrings = 0;

template <>
const QString enumStrings<PixelFormat>[5] = {
  "rgba", "index", "gray", "gray-alpha", "monochrome"
};

template <>
const QString enumStrings<LayerNameMode>[4] = {
  "automatic", "name", "index", "empty"
};

template <>
const QString enumStrings<GroupNameMode>[4] = {
  "automatic", "name", "index", "empty"
};

template <>
const QString enumStrings<FrameNameMode>[4] = {
  "automatic", "relative", "absolute", "empty"
};

template <>
const QString enumStrings<LayerVis>[3] = {
  "visible", "hidden", "all"
};

//--------------------- json helpers -----------------------------

bool getBool(QJsonObject &obj, const QString &key, const bool def) {
  const QJsonValue val = obj.take(key);
  if (val.isUndefined()) {
    return def;
  } else if (val.isBool()) {
    return val.toBool();
  } else {
    throw Error{"Field \"" + key + "\" must be a boolean"};
  }
}

int getInt(const QString &context, const QJsonValue &val) {
  const double doubleVal = val.toDouble(0.5);
  const int intVal = static_cast<int>(doubleVal);
  if (static_cast<double>(intVal) == doubleVal) {
    return intVal;
  } else {
    throw Error{context + " must be an integer"};
  }
}

QString getString(QJsonObject &obj, const QString &key, const QString &def) {
  const QJsonValue val = obj.take(key);
  if (val.isString()) {
    return val.toString();
  } else if (val.isUndefined()) {
    return def;
  } else {
    throw Error{"Field \"" + key + "\" must be a string"};
  }
}

template <typename Enum>
Enum getEnum(const QString &context, const QJsonValue val, const Enum def) {
  if (val.isUndefined()) {
    return def;
  }
  const QString str = val.toString();
  if (!str.isNull()) {
    for (std::size_t n = 0; n != std::size(enumStrings<Enum>); ++n) {
      if (enumStrings<Enum>[n] == str) {
        return static_cast<Enum>(n);
      }
    }
  }
  QString err = context + " is invalid. Valid values are:";
  for (const QString &name : enumStrings<Enum>) {
    err += "\n - ";
    err += name;
  }
  throw Error{err};
}

template <typename Enum>
Enum getEnum(QJsonObject &obj, const QString &key, const Enum def) {
  return getEnum("Field \"" + key + "\"", obj.take(key), def);
}

void checkUnused(QString err, QJsonObject &obj) {
  if (!obj.isEmpty()) {
    err += " contains unused fields:";
    for (const QString &key : obj.keys()) {
      err += "\n - ";
      err += key;
    }
    throw Error{err};
  }
}

// ----------------------------------------------------------------

QJsonDocument readDoc() {
  QByteArray array;
  while (!std::cin.eof()) {
    char buffer[1024];
    std::cin.read(buffer, sizeof(buffer));
    array.append(buffer, static_cast<int>(std::cin.gcount()));
  }
  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(array, &error);
  if (doc.isNull()) {
    throw Error{error.errorString()};
  }
  return doc;
}

std::unique_ptr<AtlasGenerator> parseGenerator(const QString &str) {
  if (str == "png") {
    return std::make_unique<PngAtlasGenerator>();
  } else if (str == "json") {
    return std::make_unique<JsonAtlasGenerator>();
  } else if (str == "cpp png") {
    return std::make_unique<CppAtlasGenerator>(DataFormat::png);
  } else if (str == "cpp raw") {
    return std::make_unique<CppAtlasGenerator>(DataFormat::raw);
  } else if (str == "cpp deflated") {
    return std::make_unique<CppAtlasGenerator>(DataFormat::deflated, false);
  } else if (str == "cpp deflated with inflate") {
    return std::make_unique<CppAtlasGenerator>(DataFormat::deflated, true);
  } else {
    QString err = "Field \"generator\" is invalid. Valid values are:";
    err += "\n - png";
    err += "\n - cpp png";
    err += "\n - cpp raw";
    err += "\n - cpp deflated";
    err += "\n - cpp deflated with inflate";
    throw Error{err};
  }
}

void setDefaultAnimation(AnimExportParams &anim, const QString &path) {
  anim.name.baseName = nameFromPath(path);
  anim.name.layerName = LayerNameMode::automatic;
  anim.name.groupName = GroupNameMode::automatic;
  anim.name.frameName = FrameNameMode::automatic;
  
  anim.transform.scaleX = 1;
  anim.transform.scaleY = 1;
  anim.transform.angle = 0;
  
  anim.layers.min = LayerIdx{0};
  anim.layers.max = LayerIdx{-1};
  anim.layers.vis = LayerVis::visible;
  
  anim.frames.min = FrameIdx{0};
  anim.frames.max = FrameIdx{-1};
  
  anim.composite = true;
}

void checkNonZeroRange(QString err, int value, const IntRange range) {
  if (value == 0 || value < range.min || value > range.max) {
    err += " must be in range [";
    err += QString::number(range.min);
    err += ", -1] U [1, ";
    err += QString::number(range.max);
    err += "]";
    throw Error{err};
  }
}

void parseScale(SpriteTransform &transform, QJsonObject &obj) {
  const QJsonValue val = obj.take("scale");
  if (val.isUndefined()) {
    transform.scaleX = 1;
    transform.scaleY = 1;
  } else if (val.isArray()) {
    const QJsonArray arr = val.toArray();
    if (arr.size() != 2) {
      throw Error{"Field \"scale\" must contain two elements"};
    }
    transform.scaleX = getInt("First element of field \"scale\"", arr[0]);
    transform.scaleY = getInt("Second element of field \"scale\"", arr[1]);
    checkNonZeroRange("First element of field \"scale\"", transform.scaleX, expt_scale);
    checkNonZeroRange("Second element of field \"scale\"", transform.scaleY, expt_scale);
  } else if (val.isDouble()) {
    transform.scaleX = transform.scaleY = getInt("Field \"scale\"", val);
    checkNonZeroRange("Field \"scale\"", transform.scaleX, expt_scale);
  } else {
    throw Error{"Field \"scale\" must an array or an integer"};
  }
}

void parseAngle(SpriteTransform &transform, QJsonObject &obj) {
  const QJsonValue val = obj.take("angle");
  if (val.isUndefined()) {
    transform.angle = 0;
  } else if (val.isDouble()) {
    transform.angle = getInt("Field \"angle\"", val);
    if (transform.angle < 0 || transform.angle > 3) {
      throw Error{"Field \"angle\" must be in the range [0, 3]"};
    }
  } else {
    throw Error{"Field \"angle\" must be a integer"};
  }
}

void parseLayers(LayerRange &range, QJsonObject &obj) {
  const QJsonValue val = obj.take("layers");
  if (val.isUndefined()) {
    range.min = LayerIdx{0};
    range.max = LayerIdx{-1};
    range.vis = LayerVis::visible;
  } else if (val.isArray()) {
    const QJsonArray arr = val.toArray();
    if (arr.size() != 2 && arr.size() != 3) {
      throw Error{"Field \"layers\" must contain two or three elements"};
    }
    range.min = LayerIdx{getInt("First element of field \"layers\"", arr[0])};
    range.max = LayerIdx{getInt("Second element of field \"layers\"", arr[1])};
    range.vis = getEnum("Third element of field \"layers\"", arr[2], LayerVis::visible);
  } else if (val.isDouble()) {
    range.min = range.max = LayerIdx{getInt("Field \"layers\"", val)};
    range.vis = LayerVis::all;
  } else {
    throw Error{"Field \"layers\" must be an array or an integer"};
  }
}

void parseFrames(FrameRange &range, QJsonObject &obj) {
  const QJsonValue val = obj.take("frames");
  if (val.isUndefined()) {
    range.min = FrameIdx{0};
    range.max = FrameIdx{-1};
  } else if (val.isArray()) {
    const QJsonArray arr = val.toArray();
    if (arr.size() != 2) {
      throw Error{"Field \"frames\" must contain two elements"};
    }
    range.min = FrameIdx{getInt("First element of field \"frames\"", arr[0])};
    range.min = FrameIdx{getInt("Second element of field \"frames\"", arr[1])};
  } else if (val.isDouble()) {
    range.min = range.max = FrameIdx{getInt("Field \"frames\"", val)};
  } else {
    throw Error{"Field \"frames\" must be an array or an integer"};
  }
}

void parseAnimation(AnimExportParams &anim, QString &path, const QJsonValue &doc) {
  if (doc.isString()) {
    path = QDir::fromNativeSeparators(doc.toString());
    setDefaultAnimation(anim, path);
  } else if (doc.isObject()) {
    QJsonObject obj = doc.toObject();
    
    if (QJsonValue val = obj.take("file"); val.isString()) {
      path = QDir::fromNativeSeparators(val.toString());
    } else {
      throw Error{"Mandatory field \"file\" must be a string"};
    }
    
    anim.name.baseName = getString(obj, "name", nameFromPath(path));
    anim.name.layerName = getEnum(obj, "layer name", LayerNameMode::automatic);
    anim.name.groupName = getEnum(obj, "group name", GroupNameMode::automatic);
    anim.name.frameName = getEnum(obj, "frame name", FrameNameMode::automatic);
    
    parseScale(anim.transform, obj);
    parseAngle(anim.transform, obj);
    parseLayers(anim.layers, obj);
    parseFrames(anim.frames, obj);
    
    anim.composite = getBool(obj, "composite", true);
    
    checkUnused("Animation object", obj);
  } else {
    throw Error{"Animation array element must be a string or an object"};
  }
}

void parseAnimationArray(
  std::vector<AnimExportParams> &anims,
  std::vector<QString> &paths,
  const QJsonArray &arr
) {
  if (arr.isEmpty()) {
    throw Error{"Animations array cannot be empty"};
  }
  for (const QJsonValue &val : arr) {
    parseAnimation(anims.emplace_back(), paths.emplace_back(), val);
  }
}

void parseParams(ExportParams &params, std::vector<QString> &paths, const QJsonDocument &doc) {
  if (!doc.isObject()) {
    throw Error{"JSON document must be an object"};
  }
  QJsonObject obj = doc.object();
  
  params.name = getString(obj, "output name", "atlas");
  params.directory = QDir::fromNativeSeparators(getString(obj, "output directory", "."));
  params.pixelFormat = getEnum(obj, "pixel format", PixelFormat::rgba);
  params.whitepixel = getBool(obj, "whitepixel", false);
  params.generator = parseGenerator(getString(obj, "generator", "png"));
  
  if (QJsonValue val = obj.take("animations"); val.isArray()) {
    parseAnimationArray(params.anims, paths, val.toArray());
  } else {
    throw Error{"Mandatory field \"animations\" must be an array"};
  }
  
  checkUnused("Document object", obj);
}

}

int cliExport(int &argc, char **argv) {
  QCoreApplication app{argc, argv};
  QTextStream console{stdout};
  
  QJsonDocument doc;
  try {
    doc = readDoc();
  } catch (Error &err) {
    console << "JSON error\n";
    console << err.msg() << '\n';
    return 1;
  }
  
  ExportParams params;
  std::vector<QString> paths;
  try {
    parseParams(params, paths, doc);
  } catch (Error &err) {
    console << "Configuration error\n";
    console << err.msg() << '\n';
    return 1;
  }

  if (Error err = exportTextureAtlas(params, paths); err) {
    console << "Export error\n";
    console << err.msg() << '\n';
    return 1;
  }
  
  return 0;
}
