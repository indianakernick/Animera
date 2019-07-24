//
//  file input widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "file input widget.hpp"

#include <QtCore/qdir.h>
#include <QtGui/qpainter.h>
#include "widget painting.hpp"
#include <QtWidgets/qcompleter.h>
#include <Qtwidgets/qfilesystemmodel.h>

FileInputWidget::FileInputWidget(QWidget *parent, const int chars)
  : FileInputWidget{parent, textBoxIconRect(chars)} {}

FileInputWidget::FileInputWidget(QWidget *parent, const TextIconRects rects)
  : TextInputWidget{parent, rects.text()},
    iconRect{rects.icon()},
    borderRect{rects.border()} {
  auto *completer = new QCompleter{this};
  auto *model = new QFileSystemModel{completer};
  model->setRootPath(QDir::rootPath());
  model->setFilter(QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot | QDir::CaseSensitive);
  completer->setModel(model);
  completer->setCompletionMode(QCompleter::InlineCompletion);
  setCompleter(completer);
  setText(QDir::rootPath());
  arrow = bakeColoredBitmap(":/General/down arrow.pbm", glob_light_2);
}

void FileInputWidget::paintEvent(QPaintEvent *event) {
  TextInputWidget::paintEvent(event);
  QPainter painter{this};
  painter.fillRect(borderRect, glob_border_color);
  painter.fillRect(iconRect.inner(), glob_main);
  painter.drawPixmap(iconRect.pos(), arrow);
}
