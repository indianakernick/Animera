//
//  file io.hpp
//  Animera
//
//  Created by Indi Kernick on 27/10/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef file_io_hpp
#define file_io_hpp

#include "error.hpp"
#include <QtCore/qfile.h>
#include <QtCore/qbuffer.h>

class FileWriter {
public:
  Error open(const QString &);
  QIODevice &dev();
  Error flush() const;
  
private:
  QBuffer buff;
  QString path;
};

class FileReader {
public:
  Error open(const QString &);
  QIODevice &dev();
  Error flush() const;

private:
  QFile file;
};

#endif
