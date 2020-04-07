//
//  file input widget.cpp
//  Animera
//
//  Created by Indiana Kernick on 24/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "file input widget.hpp"

#include "connect.hpp"
#include <QtCore/qdir.h>
#include <QtGui/qpainter.h>
#include <QtGui/qvalidator.h>
#include "widget painting.hpp"
#include "text input widget.hpp"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qcompleter.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qabstractbutton.h>
#include <QtWidgets/qfilesystemmodel.h>

class FileInputButton final : public QAbstractButton {
public:
  FileInputButton(QWidget *parent, const WidgetRect rect)
    : QAbstractButton{parent}, rect{rect} {
    setCursor(Qt::PointingHandCursor);
    arrow = bakeColoredBitmap(":/General/down arrow.png", glob_light_2);
    setFixedSize(rect.widget().size());
    setMask(QRegion{rect.outer()});
  }

private:
  QPixmap arrow;
  WidgetRect rect;

  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    paintBorder(painter, rect, glob_border_color);
    painter.fillRect(rect.inner(), glob_main);
    painter.drawPixmap(rect.pos(), arrow);
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
  text = new TextInputWidget{this, rects.text};
  icon = new FileInputButton{this, rects.icon};
  auto *layout = new QHBoxLayout{this};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(text);
  layout->addWidget(icon);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  initText();
  connectSignals();
}

QString FileInputWidget::path() const {
  return text->text();
}

void FileInputWidget::setPathFromDialog() {
  auto *dialog = new QFileDialog{parentWidget()};
  dialog->setAcceptMode(QFileDialog::AcceptOpen);
  dialog->setOption(QFileDialog::ShowDirsOnly);
  dialog->setFileMode(QFileDialog::Directory);
  dialog->setDirectory(path());
  CONNECT(dialog, fileSelected, this, setPath);
  dialog->open();
}

void FileInputWidget::setPath(const QString &newDir) {
  text->setText(QDir::cleanPath(newDir));
  changePath();
}

void FileInputWidget::simplifyPath() {
  text->setText(QDir::cleanPath(path()));
}

void FileInputWidget::changePath() {
  Q_EMIT pathChanged(path());
}

void FileInputWidget::initText() {
  auto *completer = new QCompleter{text};
  auto *model = new QFileSystemModel{completer};
  // TODO: Qt bug
  // should be model->setRootPath(QDir::rootPath())
  // https://forum.qt.io/topic/105279/update-the-qcompleter-when-calling-qlineedit-settext
  // TODO: completer suggests trailing slashes
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
  CONNECT(icon, pressed,         this, setPathFromDialog);
  CONNECT(text, editingFinished, this, simplifyPath);
  CONNECT(text, editingFinished, this, changePath);
}

#include "file input widget.moc"
