//
//  file io.cpp
//  Animera
//
//  Created by Indi Kernick on 27/10/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "file io.hpp"

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
  save dst permissions
  copy src to dst
  restore dst permissions

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

Error FileWriter::flush() const {
  QFile file{path};
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return "Failed to open file for writing";
  }
  if (!file.write(buff.data().data(), buff.data().size())) {
    // Well, fuck
    file.remove();
    return FileIOError{}.what();
  }
  return {};
}

Error FileReader::open(const QString &newPath) {
  file.setFileName(newPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::ExistingOnly)) {
    return "Failed to open file for reading";
  }
  return {};
}

QIODevice &FileReader::dev() {
  return file;
}

Error FileReader::flush() const {
  return {};
}
