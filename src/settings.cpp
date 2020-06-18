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
  return getSettings().value(key, QDir::homePath()).toString();
}
