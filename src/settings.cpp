//
//  settings.cpp
//  Animera
//
//  Created by Indi Kernick on 9/12/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "settings.hpp"

#include "connect.hpp"
#include <QtWidgets/qfiledialog.h>

QSettings &getSettings() {
  static QSettings settings;
  settings.setFallbacksEnabled(false);
  return settings;
}

void updateDirSettings(QFileDialog *dialog, const QString &key) {
  const QString dir = getSettings().value(key, QDir::homePath()).toString();
  dialog->setDirectory(dir);
  CONNECT_LAMBDA(dialog, directoryEntered, [key](const QString &dir) {
    getSettings().setValue(key, dir);
  });
}
