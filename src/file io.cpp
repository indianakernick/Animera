//
//  file io.cpp
//  Animera
//
//  Created by Indiana Kernick on 27/10/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "file io.hpp"

#include <QtCore/qdir.h>
#include "scope time.hpp"
#include "file io error.hpp"

/*
Trying to make file writes atomic

qfile::copy transfers permissions so they'd have to be restored
file rename is atomic and faster than copy

alternative a
  save dst info
  remove dst
  move src to dst
  restore dst info

alternative b
  save dst info
  copy src to dst
  restore dst info

alternative c
  src is memory
  write src to dst
*/

Error FileWriter::open(const QString &newPath) {
  buff.open(QIODevice::WriteOnly);
  path = newPath;
  return {};
}

QIODevice &FileWriter::dev() {
  return buff;
}

Error FileWriter::flush() {
  SCOPE_TIME("FileWriter::flush");
  
  // TODO: should we try to avoid opening the file twice?
  buff.close();
  const qint64 equal = filesEqual();
  return equal == buff.size() ? Error{} : flushFrom(equal);
}

qint64 FileWriter::filesEqual() const {
  SCOPE_TIME("FileWriter::filesEqual");
 
  QFile file{path};
  if (!file.exists()) return 0;
  if (file.size() != buff.size()) return 0;
  
  if (!file.open(QIODevice::ReadOnly)) return 0;
  char fileBuffer[compare_buff_size];
  qint64 equal = 0;
  
  while (!file.atEnd()) {
    const qint64 read = file.read(fileBuffer, sizeof(fileBuffer));
    if (read == -1) {
      return equal;
    }
    if (std::memcmp(fileBuffer, buff.data().data() + equal, read) != 0) {
      return equal;
    }
    equal += read;
  }
  
  return equal;
}

Error FileWriter::flushFrom(const qint64 start) const {
  SCOPE_TIME("FileWriter::flushFrom");
  
  QFile file{path};
  QIODevice::OpenMode openMode = QIODevice::WriteOnly;
  if (start != 0) openMode |= QIODevice::ReadOnly;
  if (!file.open(openMode)) {
    return file.errorString() + "\n" + QDir::toNativeSeparators(path);
  }
  
  if (start != 0 && !file.seek(start)) {
    return file.errorString();
  }
  
  const char *data = buff.data().data() + start;
  const qint64 size = buff.data().size() - start;
  const qint64 written = file.write(data, size);
  if (written != size) {
    QString err = file.errorString();
    if (written > 0) file.remove();
    return std::move(err);
  }
  
  return {};
}

Error FileReader::open(const QString &newPath) {
  file.setFileName(newPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::ExistingOnly)) {
    return file.errorString() + "\n" + QDir::toNativeSeparators(newPath);
  }
  return {};
}

QIODevice &FileReader::dev() {
  return file;
}

Error FileReader::flush() {
  file.close();
  return {};
}
