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
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include "config colors.hpp"
#include <QtGui/qvalidator.h>
#include "config geometry.hpp"
#include "widget painting.hpp"
#include "text input widget.hpp"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qabstractbutton.h>

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
      if (QDir{QString{input.data(), lastSlash}}.exists()) {
        return State::Intermediate;
      }
    }
    return State::Invalid;
  }
};

/*
Popup completer seems to work how I want the inline completer to work

Pressing backspace
  default action
  if suggestion is shown, hides suggestion
Pressing enter
  if suggestion is shown, materializes suggestion
Pressing right or left
  default action
  if suggestion is shown, materializes suggestion
Pressing up or down
  if suggestion is shown, cycles suggestions
Pressing valid path char
  default action
  show suggestion
  if the current suggestion fits the new char, it should remain
*/

/*
The inline completer for QFileSystemModel has a few problems that are difficult
to solve. There's this issue:
https://forum.qt.io/topic/105279/update-the-qcompleter-when-calling-qlineedit-settext
I managed to find a way to work around that with setCompletionPrefix. The inline
completer also doesn't support cycling through completions with the up and down
arrows. I managed to figure out how to implement that but there were more
problems. The completion wouldn't always show. There were many inconsistencies
and minor details that I didn't like and couldn't change. So once again, I find
myself rebuilding the thing from the ground up.
*/

class PathInputWidget final : public TextInputWidget {
public:
  PathInputWidget(QWidget *parent, const WidgetRect rect)
    : TextInputWidget{parent, rect} {
    setText(QDir::homePath());
    setValidator(new DirValidator{this});
    CONNECT(this, textEdited, this, updateText);
  }

private:
  QDir baseDir;
  QStringList entries;
  int index = -1;
  bool shown = false;

  void next() {
    if (shown) {
      ++index;
      if (index >= entries.size()) index = 0;
      suggest();
    } else if (!entries.isEmpty()) {
      shown = true;
      recalculateBaseDir();
      suggest();
    }
  }
  
  void prev() {
    if (shown) {
      --index;
      if (index < 0) index = entries.size() - 1;
      suggest();
    } else if (!entries.isEmpty()) {
      shown = true;
      recalculateBaseDir();
      suggest();
    }
  }
  
  static constexpr QDir::Filters filter
    = QDir::Dirs
    | QDir::Drives
    | QDir::NoDotAndDotDot
    | QDir::CaseSensitive;
  
  static constexpr QDir::SortFlags sort = QDir::Name | QDir::IgnoreCase;
  
  static QDir getBaseDir(const QString &path) {
    const int lastSlash = path.lastIndexOf('/');
    if (lastSlash == -1) {
      return QDir{QDir::rootPath(), path + "*", sort, filter};
    } else {
      return QDir{
        path.left(lastSlash + 1),
        path.right(path.size() - lastSlash - 1) + "*",
        sort, filter
      };
    }
  }
  
  void recalculateBaseDir() {
    QString path = text();
    const int lastSlash = path.lastIndexOf('/');
    QString name = lastSlash == -1 ? path : path.right(path.size() - lastSlash - 1);
    path = lastSlash == -1 ? QString{} : path.left(lastSlash + 1);
    setText(path);
    baseDir = getBaseDir(path);
    entries = baseDir.entryList();
    index = entries.indexOf(QRegularExpression{name + ".*"});
  }
  
  void suggest() {
    if (!entries.isEmpty()) {
      const int start = selectionStart();
      QString oldText = start == -1 ? text() : text().left(start);
      QString newText = baseDir.path();
      if (!newText.endsWith('/')) newText.append('/');
      if (index == -1) index = 0;
      newText.append(entries[index]);
      setText(newText);
      setSelection(oldText.size(), newText.length() - oldText.length());
    }
  }
  
  void updateText(const QString &path) {
    if (!shown) return;
    baseDir = getBaseDir(path);
    if (index != -1) {
      QString oldEntry = std::move(entries[index]);
      entries = baseDir.entryList();
      index = entries.indexOf(std::move(oldEntry));
    } else {
      entries = baseDir.entryList();
    }
    suggest();
  }

  void keyPressEvent(QKeyEvent *event) override {
    switch (event->key()) {
      case Qt::Key_Backspace:
      case Qt::Key_Delete:
      case Qt::Key_Enter:
      case Qt::Key_Left:
      case Qt::Key_Right:
        shown = false;
        break;
      case Qt::Key_Up:
        prev();
        break;
      case Qt::Key_Down:
        next();
        break;
      default:
        if (!event->text().isEmpty()) {
          shown = true;
        }
    }
    TextInputWidget::keyPressEvent(event);
  }
  
  void focusOutEvent(QFocusEvent *event) override {
    shown = false;
    TextInputWidget::focusOutEvent(event);
  }
  
  void mousePressEvent(QMouseEvent *event) override {
    shown = false;
    TextInputWidget::mousePressEvent(event);
  }
};

FileInputWidget::FileInputWidget(QWidget *parent, const int chars)
  : QWidget{parent} {
  const TextIconRects rects = textBoxIconRect(chars);
  text = new PathInputWidget{this, rects.text};
  icon = new FileInputButton{this, rects.icon};
  auto *layout = new QHBoxLayout{this};
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(text);
  layout->addWidget(icon);
  layout->setSizeConstraint(QLayout::SetFixedSize);
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
  // TODO: can the validator do this job?
  text->setText(QDir::cleanPath(path()));
}

void FileInputWidget::changePath() {
  Q_EMIT pathChanged(path());
}

void FileInputWidget::connectSignals() {
  CONNECT(icon, pressed,         this, setPathFromDialog);
  CONNECT(text, editingFinished, this, simplifyPath);
  CONNECT(text, editingFinished, this, changePath);
}

#include "file input widget.moc"
