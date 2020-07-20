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
#include "png export backend.hpp"
#include "cpp export backend.hpp"
#include "export texture atlas.hpp"
#include <QtCore/qcoreapplication.h>

namespace {

QJsonDocument readDoc(QTextStream &console) {
  QByteArray array;
  while (!std::cin.eof()) {
    char buffer[1024];
    std::cin.read(buffer, sizeof(buffer));
    array.append(buffer, static_cast<int>(std::cin.gcount()));
  }
  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(array, &error);
  if (doc.isNull()) {
    console << "JSON error\n";
    console << error.errorString() << '\n';
  }
  return doc;
}

const QString pixelFormatNames[] = {
  "rgba",
  "index",
  "gray",
  "gray-alpha",
  "monochrome"
};

template <typename Enum, std::size_t Size>
Error parseEnum(Enum &e, const QString &str, const QString &key, const QString (&names)[Size]) {
  for (std::size_t n = 0; n != std::size(names); ++n) {
    if (names[n] == str) {
      e = static_cast<Enum>(n);
      return {};
    }
  }
  QString err = "Invalid " + key + ". Valid values are:";
  for (const QString &name : names) {
    err += "\n - ";
    err += name;
  }
  return err;
}

Error parsePixelFormat(PixelFormat &format, const QString &str) {
  return parseEnum(format, str, "pixel format", pixelFormatNames);
}

Error parseBackend(std::unique_ptr<ExportBackend> &backend, const QString &str) {
  if (str == "png") {
    backend = std::make_unique<PngExportBackend>();
  } else if (str == "cpp") {
    backend = std::make_unique<CppExportBackend>();
  } else {
    QString err = "Invalid backend. Value values are:";
    err += "\n - png";
    err += "\n - cpp";
    return err;
  }
  return {};
}

template <typename ParseFunc, typename DefaultFunc>
Error getString(QJsonObject &obj, const QString &key, ParseFunc parse, DefaultFunc def) {
  QJsonValue val = obj.take(key);
  if (val.isString()) {
    if constexpr (std::is_invocable_r_v<Error, ParseFunc, QString>) {
      return parse(val.toString());
    } else {
      parse(val.toString());
      return {};
    }
  } else if (val.isUndefined()) {
    if constexpr (std::is_invocable_r_v<Error, DefaultFunc>) {
      return def();
    } else {
      def();
      return {};
    }
  } else {
    return "Field \"" + key + "\" must be a string";
  }
}

// TODO: maybe put this in export name.hpp
// this is repeated in a few places
QString getFileName(const QString &path) {
  int begin = path.lastIndexOf('/');
  int end = path.lastIndexOf('.');
  begin += 1;
  end = end == -1 ? path.size() : end;
  QString name{path.data() + begin, end - begin};
  // path.truncate(begin);
  return name;
}

void setDefaultAnimation(AnimExportParams &anim, const QString &path) {
  anim.name.baseName = getFileName(path);
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

const QString layerNames[] = {
  "automatic",
  "name",
  "index",
  "empty"
};

const QString frameNames[] = {
  "automatic",
  "relative",
  "absolute",
  "empty"
};

Error parseLayerName(LayerNameMode &name, QJsonObject &obj) {
  return getString(obj, "layer name", [&](const QString &val) {
    return parseEnum(name, val, "layer name", layerNames);
  }, [&] {
    name = LayerNameMode::automatic;
  });
}

Error parseGroupName(GroupNameMode &name, QJsonObject &obj) {
  return getString(obj, "group name", [&](const QString &val) {
    return parseEnum(name, val, "group name", layerNames);
  }, [&] {
    name = GroupNameMode::automatic;
  });
}

Error parseFrameName(FrameNameMode &name, QJsonObject &obj) {
  return getString(obj, "frame name", [&](const QString &val) {
    return parseEnum(name, val, "frame name", frameNames);
  }, [&] {
    name = FrameNameMode::automatic;
  });
}

Error parseInt(int &intVal, const QString &key, const double doubleVal) {
  intVal = static_cast<int>(doubleVal);
  if (static_cast<double>(intVal) != doubleVal) {
    return "Field \"" + key + "\" must be an integer";
  }
  return {};
}

Error parseScale(ExportTransform &transform, QJsonObject &obj) {
  const QJsonValue val = obj.take("scale");
  if (val.isUndefined()) {
    transform.scaleX = 1;
    transform.scaleY = 1;
  } else if (val.isArray()) {
    const QJsonArray arr = val.toArray();
    if (arr.size() != 2) {
      return "Field \"scale\" must contain two elements";
    }
    if (!arr[0].isDouble() || !arr[1].isDouble()) {
      return "Field \"scale\" must be an array of numbers";
    }
    // TODO: This is an odd error message
    TRY(parseInt(transform.scaleX, "scale[0]", arr[0].toDouble()));
    TRY(parseInt(transform.scaleY, "scale[1]", arr[1].toDouble()));
    if (transform.scaleX == 0) {
      return "First element of field \"scale\" cannot be zero";
    } else if (transform.scaleY == 0) {
      return "Second element of field \"scale\" cannot be zero";
    }
  } else if (val.isDouble()) {
    int intVal;
    TRY(parseInt(intVal, "scale", val.toDouble()));
    if (intVal == 0) {
      return "Field \"scale\" cannot be zero";
    }
    transform.scaleX = transform.scaleY = intVal;
  } else {
    return "Field \"scale\" must an array or a number";
  }
  return {};
}

Error parseAngle(ExportTransform &transform, QJsonObject &obj) {
  const QJsonValue val = obj.take("angle");
  if (val.isUndefined()) {
    transform.angle = 0;
  } else if (val.isDouble()) {
    int intVal;
    TRY(parseInt(intVal, "angle", val.toDouble()));
    if (intVal < 0 || intVal > 3) {
      return "Field \"angle\" must be in the range [0, 3]";
    }
    transform.angle = intVal;
  } else {
    return "Field \"angle\" must be a number";
  }
  return {};
}

const QString layerVisNames[] = {
  "visible",
  "hidden",
  "all"
};

Error parseLayers(LayerRange &range, QJsonObject &obj) {
  const QJsonValue val = obj.take("layers");
  if (val.isUndefined()) {
    range.min = LayerIdx{0};
    range.max = LayerIdx{-1};
    range.vis = LayerVis::visible;
  } else if (val.isArray()) {
    const QJsonArray arr = val.toArray();
    if (arr.size() != 2 && arr.size() != 3) {
      return "Field \"layers\" must contain two elements or three elements";
    }
    if (!arr[0].isDouble() || !arr[1].isDouble()) {
      return "First two elements of field \"layers\" must be numbers";
    }
    int min, max;
    // TODO: This is an odd error message
    TRY(parseInt(min, "layers[0]", arr[0].toDouble()));
    TRY(parseInt(max, "layers[1]", arr[1].toDouble()));
    range.min = LayerIdx{min};
    range.max = LayerIdx{max};
    if (arr.size() == 3) {
      if (!arr[2].isString()) {
        return "Third element of field \"layers\" must be a string";
      }
      TRY(parseEnum(range.vis, arr[2].toString(), "layers[2]", layerVisNames));
    } else {
      range.vis = LayerVis::visible;
    }
  } else if (val.isDouble()) {
    int intVal;
    TRY(parseInt(intVal, "layers", val.toDouble()));
    range.min = range.max = LayerIdx{intVal};
  } else {
    return "Field \"layers\" must be an array or a number";
  }
  return {};
}

Error parseFrames(FrameRange &range, QJsonObject &obj) {
  const QJsonValue val = obj.take("frames");
  if (val.isUndefined()) {
    range.min = FrameIdx{0};
    range.max = FrameIdx{-1};
  } else if (val.isArray()) {
    const QJsonArray arr = val.toArray();
    if (arr.size() != 2) {
      return "Field \"frames\" must contain two elements";
    }
    if (!arr[0].isDouble() || !arr[1].isDouble()) {
      return "Field \"frames\" must be an array of numbers";
    }
    int min, max;
    // TODO: This is an odd error message
    TRY(parseInt(min, "frames[0]", arr[0].toDouble()));
    TRY(parseInt(max, "frames[1]", arr[1].toDouble()));
    range.min = FrameIdx{min};
    range.max = FrameIdx{max};
  } else if (val.isDouble()) {
    int intVal;
    TRY(parseInt(intVal, "frames", val.toDouble()));
    range.min = range.max = FrameIdx{intVal};
  } else {
    return "Field \"frames\" must be an array or a number";
  }
  return {};
}

Error parseComposite(bool &composite, QJsonObject &obj) {
  const QJsonValue val = obj.take("composite");
  if (val.isUndefined()) {
    composite = true;
  } else if (val.isBool()) {
    composite = val.toBool();
  } else {
    return "Field \"composite\" must be a boolean";
  }
  return {};
}

Error parseAnimation(AnimExportParams &anim, QString &path, const QJsonValue &doc) {
  if (doc.isString()) {
    path = QDir::fromNativeSeparators(doc.toString());
    setDefaultAnimation(anim, path);
    return {};
  } else if (doc.isObject()) {
    QJsonObject obj = doc.toObject();
    
    if (QJsonValue val = obj.take("file"); val.isString()) {
      path = QDir::fromNativeSeparators(val.toString());
    } else {
      return "Required field \"file\" must be a string";
    }
    
    TRY(getString(obj, "name", [&](const QString &val) {
      anim.name.baseName = val;
    }, [&] {
      anim.name.baseName = getFileName(path);
    }));
    
    TRY(parseLayerName(anim.name.layerName, obj));
    TRY(parseGroupName(anim.name.groupName, obj));
    TRY(parseFrameName(anim.name.frameName, obj));
    TRY(parseScale(anim.transform, obj));
    TRY(parseAngle(anim.transform, obj));
    TRY(parseLayers(anim.layers, obj));
    TRY(parseFrames(anim.frames, obj));
    TRY(parseComposite(anim.composite, obj));
    
    if (!obj.isEmpty()) {
      QString err = "Animation object contains unused fields:";
      for (const QString &key : obj.keys()) {
        err += "\n - ";
        err += key;
      }
      return err;
    }
    
    return {};
  } else {
    return "Animation array element must be a string or an object";
  }
}

Error parseAnimationArray(
  std::vector<AnimExportParams> &anims,
  std::vector<QString> &paths,
  const QJsonArray &arr
) {
  if (arr.isEmpty()) {
    return "Animations array cannot be empty";
  }
  for (const QJsonValue &val : arr) {
    TRY(parseAnimation(anims.emplace_back(), paths.emplace_back(), val));
  }
  return {};
}

Error parseParams(ExportParams &params, std::vector<QString> &paths, const QJsonDocument &doc) {
  if (!doc.isObject()) {
    return "JSON document must be an object";
  }
  QJsonObject obj = doc.object();
  
  if (QJsonValue val = obj.take("output name"); val.isString()) {
    params.name = val.toString();
  } else {
    return "Required field \"output name\" must be a string";
  }
  
  TRY(getString(obj, "output directory", [&](const QString &val) {
    params.directory = val;
  }, [&] {
    params.directory = ".";
  }));
  
  TRY(getString(obj, "pixel format", [&](const QString &val) {
    return parsePixelFormat(params.pixelFormat, val);
  }, [&] {
    params.pixelFormat = PixelFormat::rgba;
  }));
  
  if (QJsonValue val = obj.take("whitepixel"); !val.isUndefined()) {
    if (val.isBool()) {
      params.whitepixel = val.toBool();
    } else {
      return "Field \"whitepixel\" must be a boolean";
    }
  } else {
    params.whitepixel = false;
  }
  
  TRY(getString(obj, "backend", [&](const QString &val) {
    return parseBackend(params.backend, val);
  }, [&] {
    params.backend = std::make_unique<PngExportBackend>();
  }));
  
  if (QJsonValue val = obj.take("animations"); val.isArray()) {
    TRY(parseAnimationArray(params.anims, paths, val.toArray()));
  } else {
    return "Required field \"animations\" must be an array";
  }
  
  if (!obj.isEmpty()) {
    QString err = "Document object contains unused fields:";
    for (const QString &key : obj.keys()) {
      err += "\n - ";
      err += key;
    }
    return err;
  }
  
  return {};
}

}

int cliExport(int &argc, char **argv) {
  QCoreApplication app{argc, argv};
  QTextStream console{stdout};
  QJsonDocument doc = readDoc(console);
  if (doc.isNull()) return 1;
  
  ExportParams params;
  std::vector<QString> paths;
  if (Error err = parseParams(params, paths, doc); err) {
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
