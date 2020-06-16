//
//  settings.hpp
//  Animera
//
//  Created by Indiana Kernick on 9/12/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_settings_hpp
#define animera_settings_hpp

#include <QtCore/qsettings.h>

class QFileDialog;

QSettings &getSettings();
void updateDirSettings(QFileDialog *, const QString &);

inline const QString pref_sprite_dir = "Sprite Directory";
inline const QString pref_export_dir = "Export Directory";
inline const QString pref_import_dir = "Import Directory";
inline const QString pref_palette_dir = "Palette Directory";

#endif
