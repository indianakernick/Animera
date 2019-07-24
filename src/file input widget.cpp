//
//  file input widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "file input widget.hpp"

#include <QtCore/qdir.h>
#include <QtWidgets/qcompleter.h>
#include <Qtwidgets/qfilesystemmodel.h>

FileInputWidget::FileInputWidget(QWidget *parent, const WidgetRect rect)
  : TextInputWidget{parent, rect} {
  auto *completer = new QCompleter{this};
  auto *model = new QFileSystemModel{completer};
  model->setRootPath(QDir::rootPath());
  completer->setModel(model);
  completer->setCompletionMode(QCompleter::InlineCompletion);
  setCompleter(completer);
  setText(QDir::rootPath());
}
