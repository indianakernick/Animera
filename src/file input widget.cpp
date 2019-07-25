//
//  file input widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "file input widget.hpp"

#include "connect.hpp"
#include <QtCore/qdir.h>
#include <QtGui/qpainter.h>
#include "widget painting.hpp"
#include "text input widget.hpp"
#include <QtWidgets/qcompleter.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qabstractbutton.h>
#include <Qtwidgets/qfilesystemmodel.h>

class FileInputButton final : public QAbstractButton {
public:
  explicit FileInputButton(QWidget *parent)
    : QAbstractButton{parent} {
    setCursor(Qt::PointingHandCursor);
    arrow = bakeColoredBitmap(":/General/down arrow.pbm", glob_light_2);
  }

private:
  QPixmap arrow;

  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.fillRect(rect(), glob_main);
    painter.drawPixmap(glob_text_padding, glob_text_padding, arrow);
  }
};

FileInputWidget::FileInputWidget(QWidget *parent, const int chars)
  : QWidget{parent} {
  const TextIconRects rects = textBoxIconRect(chars);
  setFixedSize(rects.widget().size());
  text = new TextInputWidget{this, rects.text()};
  icon = new FileInputButton{this};
  icon->setGeometry(rects.icon().inner());
  initText();
  CONNECT(icon, pressed, this, setTextFromDialog);
}

void FileInputWidget::initText() {
  auto *completer = new QCompleter{text};
  auto *model = new QFileSystemModel{completer};
  // @TODO bug workaround. should be QDir::rootPath()
  // https://forum.qt.io/topic/105279/update-the-qcompleter-when-calling-qlineedit-settext
  model->setRootPath(QDir::homePath());
  model->setFilter(QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot | QDir::CaseSensitive);
  completer->setModel(model);
  completer->setCompletionMode(QCompleter::InlineCompletion);
  text->setCompleter(completer);
  text->setText(QDir::homePath());
}

void FileInputWidget::setTextFromDialog() {
  QString newDir = QFileDialog::getExistingDirectory(nullptr, "", text->text());
  if (!newDir.isNull()) {
    text->setText(newDir);
  }
}
