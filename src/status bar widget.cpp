//
//  status bar widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "status bar widget.hpp"

#include <QtGui/qpainter.h>
#include <QtGui/qfontdatabase.h>

namespace {

QFont loadGlobalFont() {
  /*int id = QFontDatabase::addApplicationFont(":/Fonts/5x9 ascii.ttf");
  assert(id != -1);
  QFont font{QFontDatabase::applicationFontFamilies(id).at(0), 10};
  font.setStyleStrategy(static_cast<QFont::StyleStrategy>(QFont::PreferBitmap | QFont::NoAntialias | QFont::PreferMatch));
  font.setHintingPreference(QFont::PreferNoHinting);
  return font;*/
  QFont font{"Courier", 14};
  font.setStyleStrategy(QFont::NoAntialias);
  return font;
}

QFont getGlobalFont() {
  static QFont font = loadGlobalFont();
  return font;
}

}

StatusBarWidget::StatusBarWidget(QWidget *parent)
  : QWidget{parent} {
  timer.setInterval(5000);
  timer.setSingleShot(true);
  connect(&timer, &QTimer::timeout, this, &StatusBarWidget::hideTemp);
  setMinimumWidth(400);
  setFixedHeight(22);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  setContentsMargins(0, 0, 0, 0);
  showPerm("Hello");
  showTemp("World");
}

void StatusBarWidget::showTemp(const QString &text) {
  tempText = text;
  timer.start();
  repaint();
}

void StatusBarWidget::showPerm(const QString &text) {
  permText = text;
  repaint();
}

void StatusBarWidget::paintEvent(QPaintEvent *) {
  if (textImg.size() != size() / 2) {
    textImg = QPixmap{size() / 2};
  }
  QPainter textPainter{&textImg};
  textPainter.fillRect(rect(), {127, 127, 127});
  textPainter.setPen(QColor{255, 255, 255});
  textPainter.setFont(getGlobalFont());
  if (tempText.isEmpty()) {
    textPainter.drawText(1, 10, permText);
  } else {
    textPainter.drawText(1, 10, permText + " | " + tempText);
  }
  textPainter.end();
  
  QPainter painter{this};
  painter.drawPixmap(rect(), textImg);
}

void StatusBarWidget::hideTemp() {
  tempText = "";
  repaint();
}

#include "status bar widget.moc"
