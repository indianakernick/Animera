//
//  cli.cpp
//  Animera
//
//  Created by Indi Kernick on 23/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "cli.hpp"

#include "sprite.hpp"
#include "application.hpp"
#include <QtCore/qtextstream.h>
#include <QtCore/qcommandlineparser.h>

CLI::CLI(int &argc, char **argv)
  : argc{argc}, argv{argv} {}

int CLI::exec() {
  QCommandLineParser parser;

  const QCommandLineOption name{"name", "", "name"};
  const QCommandLineOption dir{"directory", "", "directory"};
  parser.addOption(name);
  parser.addOption(dir);
  const QCommandLineOption help = parser.addHelpOption();
  
  if (!parser.parse(getArguments())) {
    console() << parser.errorText() << '\n';
    return 1;
  }
  
  if (parser.isSet(help)) {
    printHelp();
    return 0;
  }
  
  const QStringList &args = parser.positionalArguments();
  if (args.size() == 1) {
    console() << "Missing file argument\n";
    return 1;
  }
  if (args.size() > 2) {
    console() << "Too many arguments\n";
    return 1;
  }
  
  if (args.isEmpty()) {
    return execDefault();
  }
  if (args.front() == "open") {
    return execOpen(args.back());
  }
  if (args.front() == "export") {
    return execExport(args.back(), parser.value(name), parser.value(dir));
  }
  
  console() << "Invalid arguments\n";
  return 1;
}

namespace {

const char helpText[] =
R"(Usage:
  %s
  %s open <file>
  %s export <file> [--name --directory]

Options:
  --name <pattern>   Name pattern for the sprite.
                     Overrides name in config file.
  --directory <dir>  Directory to write files to.
                     Overrides directory in config file.
  -h, --help         Displays this help.)";

}

/*

{
  "name": "sprite_%000F",
  "directory": ".",
  "layer-line": [1, 0],
  "frame-line": [1, 0],
  "layer": 5,              default is all layers
  "composited": true,
  "frame": 0,              default is all frames
  "format": "rgba",        rgba, index, gray, gray-alpha, monochome
  "scale": [1, 1],
  "angle": 0               0,1,2,3
}

layer visibility

*/

void CLI::printHelp() const {
  std::printf(helpText, argv[0], argv[0], argv[0]);
}

QStringList CLI::getArguments() const {
  QStringList arguments;
  for (int a = 0; a != argc; ++a) {
    arguments.append(argv[a]);
  }
  return arguments;
}

QTextStream &CLI::console() {
  static QTextStream stream{stdout};
  return stream;
}

int CLI::execDefault() const {
  Application app{argc, argv};
  app.waitForOpenEvent();
  return app.exec();
}

int CLI::execOpen(const QString &file) const {
  Application app{argc, argv};
  app.openFile(file);
  return app.exec();
}

int CLI::execExport(const QString &file, const QString &name, const QString &dir) const {
  QCoreApplication app{argc, argv};
  Sprite sprite;
  if (Error err = sprite.openFile(file); err) {
    console() << "File open error\n";
    console() << err.msg() << '\n';
  }
  const ExportOptions options = exportFrameOptions(dir + "/" + name + ".png", sprite.getFormat());
  if (Error err = sprite.exportSprite(options); err) {
    console() << "Export error\n";
    console() << err.msg() << '\n';
  }
  return 0;
}
