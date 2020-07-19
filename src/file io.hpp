//
//  file io.hpp
//  Animera
//
//  Created by Indiana Kernick on 27/10/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_file_io_hpp
#define animera_file_io_hpp

#include "error.hpp"
#include <QtCore/qfile.h>
#include <QtCore/qbuffer.h>

class FileWriter {
public:
  static constexpr std::size_t compare_buff_size = 4 * 1024;

  Error open(const QString &);
  QIODevice &dev();
  Error flush() const;
  
private:
  QBuffer buff;
  QString path;
  
  qint64 filesEqual() const;
  Error flushFrom(qint64) const;
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
