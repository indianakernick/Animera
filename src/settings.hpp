//
//  settings.hpp
//  Animera
//
//  Created by Indiana Kernick on 9/12/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef settings_hpp
#define settings_hpp

#include <QtCore/qsettings.h>

class QFileDialog;

QSettings &getSettings();
void updateDirSettings(QFileDialog *, const QString &);

#endif
