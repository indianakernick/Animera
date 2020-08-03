//
//  cli.cpp
//  Animera
//
//  Created by Indiana Kernick on 23/11/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "cli.hpp"

#include "strings.hpp"
#include "cli new.hpp"
#include "cli info.hpp"
#include "cli export.hpp"
#include "scope time.hpp"
#include "application.hpp"
#include <QtCore/qtextstream.h>

namespace {

const char usage[] =
R"(Usage:
    Animera
    Animera --help
    Animera --long-help
    Animera new <width> <height> [<format>]
    Animera open <file>
    Animera info [--groups --layers --json] <file>
    Animera export)";

const char short_options[] =
R"(Options:
    -h, --help                Display this help message.
    --long-help               Display a detailed help message.
    <width>                   Width of the animation to create.
    <height>                  Height of the animation to create.
    <format>                  Format of the animation to create.
    <file>                    Animation file to open.
    --groups                  Output info about groups.
    --layers                  Output info about layers.
    -j, --json                Output info as JSON.)";

const char long_options[] =
R"(Options:
    -h, --help
        Display a brief help message.
    
    --long-help
        Display this help message.
    
    <width>
        Width (in pixels) of the animation to create.
    
    <height>
        Height (in pixels) of the animation to create.
    
    <file>
        Animation file to open.
    
    <format>
        Format of the animation to create. Valid formats are:
            rgba   (8-bit RGBA)
            index  (8-bit Indexed)
            gray   (8-bit Grayscale with alpha)
        This is "rgba" by default.
    
    --groups
        Outputs information about the groups. This includes the name, the
        starting position, and the length.
    
    --layers
        Outputs information about the layers. This includes the name, the number
        of cels and the visibility.
    
    -j, --json
        By default, animation info is outputted in a pleasant-for-humans format.
        When this option is present, animation info is outputted as JSON.

Export Command
    The export command expects a JSON document as stdin. This document contains
    parameters for the atlas, as well as parameters for each animation that make
    up the atlas. There's are a lot of parameters to tune but almost all of them
    have defaults. The simplest configuration looks like this:
    
    { "animations": ["path/to/file.animera"] }
    
    The "output name" field specifies the name of the atlas file (or files) to
    create. For example, the below configuration generates a my-atlas.cpp file
    and a my-atlas.hpp file.
    
    {
      "output name": "my-atlas",
      "generator": "cpp png"
      "animations": ["path/to/file.animera"]
    }
    
    The "output directory" field specifies the directory to place the atlas
    files into. Using the above example, this is the directory to place
    my-atlas.cpp and my-atlas.hpp.
    
    The "pixel format" field specifies the pixel format of the texture to
    generate. The list of valid values for this field are below.
    
    "pixel format" field:
     - "rgba"        (8-bit RGBA)
     - "index"       (8-bit Indexed)
     - "gray"        (8-bit Grayscale)
     - "gray-alpha"  (8-bit Grayscale with alpha)
     - "monochrome"  (1-bit Grayscale)
    
    Not all generators support all pixel formats and not all pixel formats are
    compatible with all animation formats.
    
    The "whitepixel" field specifies whether to include a white pixel. That is,
    a sprite that's just a single white pixel. This is can be used to render
    solid, untextured polygons through a textured pipeline.
    
    The "generator" field specifies which texture atlas generator to use. The
    list of valid values for this field are below.
    
    "generator" field:
     - "png"  (multiple individual png files)
     - "cpp png"  (a cpp and hpp file with embedded png)
     - "cpp raw"  (a cpp and hpp file with embedded uncompressed image data)
     - "cpp deflated"  (a cpp and hpp file with embedded deflated image data)
     - "cpp deflated with inflate"  (a cpp and hpp file with embedded deflated
       image data and inflate function)
    
    Those are the parameters for the atlas. The "simplest configuration" from
    earlier is equivalent to this:
    
    {
      "output name": "atlas",
      "output directory": ".",
      "pixel format": "rgba",
      "whitepixel": false,
      "generator": "png",
      "animations": ["path/to/file.animera"]
    }
    
    The "animations" field is an array containing all the animations to include
    and parameters for each of them. The array can contain objects or strings.
    A string can be used as shorthand for an object with the "file" field set.
    The "file" field is the only mandatory field and it specifies the animation
    file to export.
    
    The "name" field specifies the base name to use for all sprites in the file.
    The default name is the file name. So if "file": "path/to/player.animera",
    then "name": "player" by default. The name can be an empty string.
    
    The "layer name" field specifies how the layer of the sprite should be used
    in the sprite name. Valid values are below.
    
    "layer name" field:
     - "automatic"  (empty if there is only one layer
                     otherwise layer name if it's not empty
                     otherwise layer index)
     - "name"       (layer name is appended)
     - "index"      (layer index is appended)
     - "empty"      (nothing is appended)
    
    The "group name" field is very similar to the "layer name" field except that
    it applies to groups. Its valid values are the same as for "layer name".
    
    The "frame name" field is similar but different to the previous name fields.
    It has a different set of valid values.
    
    "frame name" field:
     - "automatic" (relative frame index if there is more than one frame)
     - "relative"  (frame index relative to group is appended)
     - "absolute"  (absolute frame index is appended)
     - "empty"     (nothing is appended)
    
    The "scale" field specifies the scale factor to apply to each sprite in the
    animation. It can either be a single integer to uniformly scale the sprite,
    or an array of two integers to set the scale factor for the width and height
    separately. The scale factor must be a non-zero integer and can be negative
    to indicate flipping.
    
    The "angle" field specifies an angle to rotate each sprite in the animation.
    The angle must be an integer between 0 and 3 that specifies the number of 90
    degree rotations to perform.
    
    The "layers" field specifies the range of layers to include. It can an
    integer to select a single layer. It can also be an array of two integers to
    select a minimum and maximum layer. The array may also contain a third
    element that is a string. This element specifies the visibility of the
    layers to select. Valid values are below. The layer indicies may be negative
    to select layers relative to the last layer. So -1 is the last layer and -2
    is one before the last. This makes it possible to select all the layers with
    "layers": [0, -1, "all"].
    
    Third element of "layers" field:
     - "visible"  (the visible layers within the selection)
     - "hidden"   (the hidden layers within the selection)
     - "all"      (all layers within the selection)
    
    The "frames" field is similar to the "layers" field except that it applies
    to frames and doesn't allow for a third element in its array form.
    
    The "composite" field specifies whether the layers of the animation should
    be composited into a single layer or left as separate layers.
    
    The "simplest configuration" from earlier can also be written with all the
    animation parameters set as well. The below configuration is equivalent to
    the one-liner above.
    
    {
      "output name": "atlas",
      "output directory": ".",
      "pixel format": "rgba",
      "whitepixel": false,
      "generator": "png",
      "animations": [
        {
          "file": "path/to/file.animera",
          "name": "file",
          "layer name": "automatic",
          "group name": "automatic",
          "frame name": "automatic",
          "scale": [1, 1],
          "angle: 0,
          "layers": [0, -1, "visible"],
          "frames": [0, -1],
          "composite": true
        }
      ]
    })";

}

CLI::CLI(int &argc, char **argv)
  : argc{argc}, argv{argv} {}

int CLI::exec() {
  SCOPE_TIME("CLI::exec");

  docopt::Options flags;
  QTextStream console{stdout};
  if (Error err = parseArgs(flags); err) {
    console << "Command line error\n" << err.msg() << '\n';
    console << usage << '\n';
    return 1;
  }
  
  if (flags.at("--help").asBool()) {
    console << usage << "\n\n" << short_options << '\n';
    return 0;
  } else if (flags.at("--long-help").asBool()) {
    console << usage << "\n\n" << long_options << '\n';
    return 0;
  } else if (flags.at("new").asBool()) {
    return cliNew(argc, argv, flags);
  } else if (flags.at("open").asBool()) {
    return execOpen(flags);
  } else if (flags.at("info").asBool()) {
    return cliInfo(argc, argv, flags);
  } else if (flags.at("export").asBool()) {
    return cliExport(argc, argv);
  } else {
    return execDefault();
  }
}

Error CLI::parseArgs(docopt::Options &flags) const {
  SCOPE_TIME("CLI::parseArgs");

  // https://stackoverflow.com/a/10242200/4093378
  char **first = argv + 1;
  if (argc == 2 && std::strncmp(*first, "-psn_", 5) == 0) {
    ++first;
  }
  std::string doc = usage;
  doc += "\n\n";
  doc += short_options;
  try {
    flags = docopt::docopt_parse(doc, {first, argv + argc}, false, false);
  } catch (docopt::DocoptArgumentError &e) {
    return e.what();
  }
  return {};
}

int CLI::execDefault() const {
  Application app{argc, argv};
  app.waitForOpenEvent();
  return app.exec();
}

int CLI::execOpen(const docopt::Options &flags) const {
  Application app{argc, argv};
  app.openFile(toLatinString(flags.at("<file>").asString()));
  return app.exec();
}
