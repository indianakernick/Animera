//
//  cli info.cpp
//  Animera
//
//  Created by Indiana Kernick on 11/4/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "cli info.hpp"

#include "file io.hpp"
#include "strings.hpp"
#include "sprite file.hpp"
#include <QtCore/qtextstream.h>
#include <QtCore/qcoreapplication.h>

namespace {

Error readInfo(const QString &path, SpriteInfo &info) {
  FileReader reader;
  TRY(reader.open(path));
  TRY(readSignature(reader.dev()));
  TRY(readAHDR(reader.dev(), info));
  TRY(reader.flush());
  return Error{};
}

QString formatToString(const Format format) {
  switch (format) {
    case Format::rgba:
      return "RGBA";
    case Format::index:
      return "Indexed";
    case Format::gray:
      return "Grayscale";
    default:
      return "";
  }
}

}

int cliInfo(int &argc, char **argv, const std::map<std::string, docopt::value> &flags) {
  QCoreApplication app{argc, argv};
  QTextStream console{stdout};
  SpriteInfo info;
  
  if (Error err = readInfo(toLatinString(flags.at("<file>").asString()), info); err) {
    console << "File open error\n";
    console << err.msg() << '\n';
    return 1;
  }
  
  // TODO: Maybe an option to output LHDR
  
  if (flags.at("--json").asBool()) {
    console << "{\n";
    console << "  \"width\": " << info.width << ",\n";
    console << "  \"height\": " << info.height << ",\n";
    console << "  \"format\": \"" << formatToString(info.format) << "\",\n";
    console << "  \"layers\": " << static_cast<int>(info.layers) << ",\n";
    console << "  \"frames\": " << static_cast<int>(info.frames) << ",\n";
    console << "  \"delay\": " << info.delay << '\n';
    console << "}\n";
  } else {
    console << "Size:   {" << info.width << ", " << info.height << "}\n";
    console << "Format: " << formatToString(info.format) << '\n';
    console << "Layers: " << static_cast<int>(info.layers) << '\n';
    console << "Frames: " << static_cast<int>(info.frames) << '\n';
    console << "Delay:  " << info.delay << " ms\n";
  }
  
  return 0;
}
