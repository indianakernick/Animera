//
//  cli new.cpp
//  Animera
//
//  Created by Indiana Kernick on 11/4/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "cli new.hpp"

#include "config.hpp"
#include "application.hpp"
#include "docopt helpers.hpp"
#include <QtCore/qtextstream.h>

namespace {

const char *formatNames[] = {
  "rgba",
  "index",
  "gray"
};

Error setFormatSize(Format &format, QSize &size, const docopt::Options &flags) {
  TRY(setInt(size.rwidth(), flags.at("<width>"), "width", init_size_range));
  TRY(setInt(size.rheight(), flags.at("<height>"), "height", init_size_range));
  if (const docopt::value &value = flags.at("<format>"); value) {
    if (!setEnum(format, value.asString(), formatNames)) {
      return "Invalid format" + validListStr("formats", formatNames);
    }
  } else {
    format = Format::rgba;
  }
  return {};
}

}

int cliNew(int &argc, char **argv, const docopt::Options &flags) {
  QTextStream console{stdout};
  
  Format format;
  QSize size;
  if (Error err = setFormatSize(format, size, flags); err) {
    console << "Configuration error\n";
    console << err.msg() << '\n';
    return 1;
  }
  
  Application app{argc, argv};
  app.newFile(format, size);
  return app.exec();
}
