//
//  global font.cpp
//  Animera
//
//  Created by Indiana Kernick on 29/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "global font.hpp"

#include "config geometry.hpp"
#include <QtGui/qfontdatabase.h>

namespace {

QFont loadGlobalFont() {
  int id = QFontDatabase::addApplicationFont(":/Fonts/5x7ascii.ttf");
  assert(id != -1);
  QFont font{QFontDatabase::applicationFontFamilies(id).at(0), glob_font_pt};
  font.setStyleStrategy(QFont::NoAntialias);
  return font;
}

}

QFont getGlobalFont() {
  static QFont font = loadGlobalFont();
  return font;
}
