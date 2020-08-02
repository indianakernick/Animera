//
//  settings.cpp
//  Animera
//
//  Created by Indiana Kernick on 9/12/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "settings.hpp"

#include <iostream>
#include "connect.hpp"
#include <QtWidgets/qfiledialog.h>

void SetDirFunctor::operator()(const QString &dir) const {
  getSettings().setValue(key, dir);
}

QSettings &getSettings() {
  static QSettings settings;
  static bool init = false;
  if (!std::exchange(init, true)) {
    settings.setFallbacksEnabled(false);
    std::cout << "Settings file: " << settings.fileName().toStdString() << '\n';
  }
  return settings;
}

void updateDirSettings(QFileDialog *dialog, const QString &key) {
  dialog->setDirectory(getDirSettings(key));
  CONNECT_LAMBDA(dialog, directoryEntered, setDirSettings(key));
}

QString getDirSettings(const QString &key) {
  QString dir = getSettings().value(key, QString{}).toString();
  while (!dir.isEmpty() && !QDir{dir}.exists()) {
    dir.truncate(dir.lastIndexOf('/'));
  }
  if (dir.isEmpty()) {
    dir = QDir::homePath();
  }
  getSettings().setValue(key, dir);
  return dir;
}

SetDirFunctor setDirSettings(const QString &key) {
  return {key};
}
