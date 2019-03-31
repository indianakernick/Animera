//
//  cursors.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 31/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "cursors.hpp"

#include "config.hpp"
#include <QtGui/qbitmap.h>

namespace {

QCursor loadCursor(const QString &name) {
  return {
    QBitmap{":/Cursors/" + name + " b.pbm"}.scaled(glob_cursor_size),
    QBitmap{":/Cursors/" + name + " m.pbm"}.scaled(glob_cursor_size),
    glob_cursor_offset.x(),
    glob_cursor_offset.y()
  };
}

}

QCursor getCircleCursor() {
  static QCursor cursor = loadCursor("circle");
  return cursor;
}
