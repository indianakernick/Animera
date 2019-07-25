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
#include <QtGui/qvalidator.h>
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

class DirValidator final : public QValidator {
public:
  explicit DirValidator(QWidget *parent)
    : QValidator{parent} {}
  
  void fixup(QString &input) const override {
    while (!input.isEmpty() && !QDir{input}.exists()) {
      input.truncate(input.lastIndexOf(QDir::separator()));
    }
    if (input.isEmpty()) {
      input = QDir::rootPath();
    }
  }
  
  State validate(QString &input, int &) const override {
    if (input.isEmpty()) {
      return State::Intermediate;
    }
    if (QDir{input}.exists()) {
      return State::Acceptable;
    }
    const int lastSlash = input.lastIndexOf(QDir::separator());
    if (lastSlash != -1) {
      const QString truncated{input.data(), lastSlash};
      if (QDir{truncated}.exists()) {
        return State::Intermediate;
      }
    }
    return State::Invalid;
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
  connectSignals();
}

void FileInputWidget::setTextFromDialog() {
  QString newDir = QFileDialog::getExistingDirectory(nullptr, "", text->text());
  if (!newDir.isNull()) {
    text->setText(QDir::cleanPath(newDir));
  }
}

void FileInputWidget::simplifyPath() {
  text->setText(QDir::cleanPath(text->text()));
}

void FileInputWidget::changePath() {
  Q_EMIT pathChanged(text->text());
}

void FileInputWidget::initText() {
  auto *completer = new QCompleter{text};
  auto *model = new QFileSystemModel{completer};
  // @TODO bug workaround. should be QDir::rootPath()
  // https://forum.qt.io/topic/105279/update-the-qcompleter-when-calling-qlineedit-settext
  // @TODO completer suggests trailing slashes
  // these are later removed by cleanPath.
  // pressing slash while completer is suggesting something should materialize
  // the suggestion, append a slash and suggest another directory
  
  // Custom DirCompleter would solve both problems
  model->setRootPath(QDir::homePath());
  model->setFilter(QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot | QDir::CaseSensitive);
  completer->setModel(model);
  completer->setCompletionMode(QCompleter::InlineCompletion);
  text->setCompleter(completer);
  validator = new DirValidator{text};
  text->setValidator(validator);
  text->setText(QDir::homePath());
}

void FileInputWidget::connectSignals() {
  CONNECT(icon, pressed,         this, setTextFromDialog);
  CONNECT(text, editingFinished, this, simplifyPath);
  CONNECT(text, editingFinished, this, changePath);
}

#include "file input widget.moc"
