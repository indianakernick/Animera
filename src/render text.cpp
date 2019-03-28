//
//  render text.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "render text.hpp"

#include "config.hpp"
#include <QtCore/qfile.h>
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include <Simpleton/Sprite/sheet.hpp>

namespace {

QPixmap loadTex() {
  QPixmap tex{":/Fonts/5x9 ascii.png"};
  tex.setMask(tex.createMaskFromColor(QColor{}));
  return tex;
}

Sprite::Sheet loadAtlas() {
  QFile file{":/Fonts/5x9 ascii.atlas"};
  [[maybe_unused]] const bool success = file.open(QFile::ReadOnly);
  assert(success);
  QByteArray bytes = file.readAll();
  return Sprite::makeSheetFromData(bytes.data(), bytes.size());
}

QPixmap getTex() {
  static QPixmap tex = loadTex();
  return tex;
}

const Sprite::Sheet &getAtlas() {
  static Sprite::Sheet atlas = loadAtlas();
  return atlas;
}

QRect lookupChar(const Sprite::Sheet &sheet, const char c) {
  assert('!' <= c && c <= '~');
  Sprite::Rect sprite = sheet.getSprite(c - '!');
  const glm::vec2 length(sheet.getLength());
  sprite.min *= length;
  sprite.max *= length;
  return QRect{
    QPoint(std::round(sprite.min.x),     std::round(sprite.max.y)),
    QPoint(std::round(sprite.max.x) - 1, std::round(sprite.min.y) - 1)
  };
}

}

void renderText(QPainter &painter, int x, int y, const std::string &text) {
  QPixmap tex = getTex();
  const Sprite::Sheet &atlas = getAtlas();
  for (const char c : text) {
    if ('!' <= c && c <= '~') {
      painter.drawPixmap(QRect{x, y, 5 * glob_scale, 9 * glob_scale}, tex, lookupChar(atlas, c));
    }
    x += (5 + 1) * glob_scale;
  }
}
