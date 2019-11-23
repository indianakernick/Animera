//
//  cli.hpp
//  Animera
//
//  Created by Indi Kernick on 23/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef cli_hpp
#define cli_hpp

class QString;
class QStringList;
class QTextStream;

class CLI {
public:
  CLI(int &, char **);

  int exec();

private:
  int &argc;
  char **argv;
  
  void printHelp() const;
  QStringList getArguments() const;
  static QTextStream &console();
  
  int execDefault() const;
  int execOpen(const QString &) const;
  int execExport(const QString &, const QString &, const QString &) const;
};

#endif
