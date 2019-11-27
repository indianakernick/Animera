//
//  cli.hpp
//  Animera
//
//  Created by Indi Kernick on 23/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef cli_hpp
#define cli_hpp

#include "docopt.h"

class CLI {
public:
  CLI(int &, char **);

  int exec();

private:
  int &argc;
  char **argv;
  
  int execDefault() const;
  int execOpen(const std::map<std::string, docopt::value> &) const;
  int execExport(const std::map<std::string, docopt::value> &) const;
};

#endif
