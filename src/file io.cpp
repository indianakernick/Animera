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

Error FileWriter::flush() const {
  SCOPE_TIME("FileWriter::flush");

  QFile file{path};
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return file.errorString() + "\n" + QDir::toNativeSeparators(path);
  }
  const qint64 written = file.write(buff.data().data(), buff.data().size());
  if (written != buff.data().size()) {
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

Error FileReader::flush() const {
  return {};
}
