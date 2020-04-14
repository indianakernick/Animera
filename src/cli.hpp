//
//  cli.hpp
//  Animera
//
//  Created by Indiana Kernick on 23/11/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_cli_hpp
#define animera_cli_hpp

#include <docopt.h>
#include "error.hpp"

class CLI {
public:
  CLI(int &, char **);

  int exec();

private:
  int &argc;
  char **argv;
  
  Error parseArgs(std::map<std::string, docopt::value> &) const;
  int execDefault(const std::map<std::string, docopt::value> &) const;
  int execOpen(const std::map<std::string, docopt::value> &) const;
};

#endif
