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
  : TextInputWidget{parent, {chars, 0, glob_box_button_icon_width}},
    rect{chars, 0, glob_box_button_icon_width} {
  auto *completer = new QCompleter{this};
  auto *model = new QFileSystemModel{completer};
  model->setRootPath(QDir::rootPath());
  completer->setModel(model);
  completer->setCompletionMode(QCompleter::InlineCompletion);
  setCompleter(completer);
  setText(QDir::rootPath());
  arrow = bakeColoredBitmap(":/General/down arrow.pbm", glob_light_2);
  assert(arrow.width() == glob_box_button_icon_width);
}

void FileInputWidget::paintEvent(QPaintEvent *event) {
  TextInputWidget::paintEvent(event);
  QPainter painter{this};
  painter.fillRect(rect.border(), glob_border_color);
  painter.fillRect(rect.buttonInner(), glob_main);
  painter.drawPixmap(rect.buttonPos(), arrow);
}
