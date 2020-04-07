//
//  timeline painting.cpp
//  Animera
//
//  Created by Indiana Kernick on 31/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "timeline painting.hpp"

#include "config.hpp"
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>

void paintTimelineButtonBack(QPixmap &pixmap) {
  QPainter painter{&pixmap};
  painter.fillRect(
    0, 0,
    cell_width - glob_border_width, cell_width - glob_border_width,
    glob_main
  );
  painter.fillRect(
    cell_width - glob_border_width, 0,
    glob_border_width, cell_width,
    glob_border_color
  );
  painter.fillRect(
    0, cell_width - glob_border_width,
    cell_width - glob_border_width, glob_border_width,
    glob_border_color
  );
}

void paintTimelineButtonIcon(QPixmap &pixmap, const QString &path) {
  QPainter painter{&pixmap};
  QBitmap bitmap{path};
  bitmap = bitmap.scaled(bitmap.size() * glob_scale);
  QRegion region{bitmap};
  region.translate(cell_icon_pad, cell_icon_pad);
  painter.setClipRegion(region);
  painter.fillRect(
    cell_icon_pad, cell_icon_pad,
    cell_icon_size, cell_icon_size,
    cell_icon_color
  );
}
