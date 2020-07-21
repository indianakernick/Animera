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
#include "chunk io.hpp"
#include "cel array.hpp"
#include "animation file.hpp"
#include <QtCore/qtextstream.h>
#include <QtCore/qcoreapplication.h>

namespace {

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

QString formatToJsonString(const Format format) {
  // These format names are used by the other commands (new)
  switch (format) {
    case Format::rgba:
      return "rgba";
    case Format::index:
      return "index";
    case Format::gray:
      return "gray";
    default:
      return "";
  }
}

void writeEscaped(QTextStream &console, const std::string_view text) {
  for (const char ch : text) {
    if (ch == '\\') {
      console << "\\\\";
    } else if (ch == '"') {
      console << "\\\"";
    } else {
      console << ch;
    }
  }
}

Error printGroups(QTextStream &console, QIODevice &dev, const AnimationInfo &info, const bool json) try {
  ChunkReader reader{dev};
  reader.skip(reader.peek());
  std::vector<Group> groups(static_cast<int>(info.groups));
  TRY(readGRPS(dev, groups, info.frames));
  FrameIdx start{};
  
  for (std::size_t g = 0; g != groups.size(); ++g) {
    if (json) {
      if (g == 0) {
        console << "    ";
      } else {
        console << ", ";
      }
      console << "{\n";
      console << "      \"name\": \"";
      writeEscaped(console, groups[g].name);
      console << "\",\n";
      console << "      \"start\": " << static_cast<int>(start) << ",\n";
      console << "      \"length\": " << static_cast<int>(groups[g].end - start) << '\n';
      console << "    }";
    } else {
      console << "  Group " << g << ":\n";
      console << "    Name:   " << toLatinString(groups[g].name) << '\n';
      console << "    Start:  " << static_cast<int>(start) << '\n';
      console << "    Length: " << static_cast<int>(groups[g].end - start) << '\n';
    }
    start = groups[g].end;
  }
  
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

Error printLayers(QTextStream &console, QIODevice &dev, const bool json) try {
  ChunkReader reader{dev};
  int index = 0;
  Layer layer;
  
  while (!dev.atEnd()) {
    const ChunkStart start = reader.peek();
    if (std::memcmp(start.name, chunk_layer_header, chunk_name_len) != 0) {
      reader.skip(start);
      continue;
    }
    TRY(readLHDR(dev, layer));
  
    if (json) {
      if (index == 0) {
        console << "    ";
      } else {
        console << ", ";
      }
      console << "{\n";
      console << "      \"name\": \"";
      writeEscaped(console, layer.name);
      console << "\",\n";
      console << "      \"cels\": " << layer.cels.size() << ",\n";
      console << "      \"visible\": " << (layer.visible ? "true" : "false") << '\n';
      console << "    }";
    } else {
      console << "  Layer " << index << ":\n";
      console << "    Name:    " << toLatinString(layer.name) << '\n';
      console << "    Cels:    " << layer.cels.size() << '\n';
      console << "    Visible: " << (layer.visible ? "Yes" : "No") << '\n';
    }
    
    ++index;
  }
  
  return {};
} catch (FileIOError &e) {
  return e.msg();
}

Error printInfo(const QString &path, const bool groups, const bool layers, const bool json) {
  QTextStream console{stdout};
  FileReader reader;
  TRY(reader.open(path));
  
  AnimationInfo anim;
  TRY(readSignature(reader.dev()));
  TRY(readAHDR(reader.dev(), anim));
  
  if (json) {
    console << "{\n";
    console << "  \"width\": " << anim.width << ",\n";
    console << "  \"height\": " << anim.height << ",\n";
    console << "  \"format\": \"" << formatToJsonString(anim.format) << "\",\n";
    console << "  \"layers\": " << static_cast<int>(anim.layers) << ",\n";
    console << "  \"groups\": " << static_cast<int>(anim.groups) << ",\n";
    console << "  \"frames\": " << static_cast<int>(anim.frames) << ",\n";
    console << "  \"delay\": " << anim.delay;
  } else {
    console << "Size:   {" << anim.width << ", " << anim.height << "}\n";
    console << "Format: " << formatToString(anim.format) << '\n';
    console << "Layers: " << static_cast<int>(anim.layers) << '\n';
    console << "Groups: " << static_cast<int>(anim.groups) << '\n';
    console << "Frames: " << static_cast<int>(anim.frames) << '\n';
    console << "Delay:  " << anim.delay << " ms\n";
  }
  
  if (groups) {
    if (json) {
      console << ",\n  \"groups\": [\n";
      TRY(printGroups(console, reader.dev(), anim, true));
      console << "\n  ]";
    } else {
      console << "Groups:\n";
      TRY(printGroups(console, reader.dev(), anim, false));
    }
  }
  
  if (layers) {
    if (json) {
      console << ",\n  \"layers\": [\n";
      TRY(printLayers(console, reader.dev(), true));
      console << "\n  ]";
    } else {
      console << "Layers:\n";
      TRY(printLayers(console, reader.dev(), false));
    }
  }
  
  if (json) {
    console << "\n}\n";
  }
  
  TRY(reader.flush());
  return Error{};
}

}

int cliInfo(int &argc, char **argv, const docopt::Options &flags) {
  QCoreApplication app{argc, argv};
  QTextStream console{stdout};
  
  const QString path = toLatinString(flags.at("<file>").asString());
  const bool groups = flags.at("--groups").asBool();
  const bool layers = flags.at("--layers").asBool();
  const bool json = flags.at("--json").asBool();
  
  if (Error err = printInfo(path, groups, layers, json); err) {
    console << "File open error\n";
    console << err.msg() << '\n';
    return 1;
  }
  
  return 0;
}
