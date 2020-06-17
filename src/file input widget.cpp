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
  
  State validate(QString &input, int &pos) const override {
    QDir::toNativeSeparators(input);
    if (input.isEmpty()) {
      input = QDir::rootPath();
      pos = input.size();
      return State::Intermediate;
    }
    const int colon = input.indexOf(':');
    if (colon == 0 || colon > 1) {
      return State::Invalid;
    }
    if (QDir::isRelativePath(input)) {
      return State::Invalid;
    }
    if (QDir{input}.exists()) {
      return State::Acceptable;
    }
    const int slash = input.lastIndexOf(QDir::separator());
    if (slash != -1 && QDir{input.left(slash)}.exists()) {
      return State::Intermediate;
    }
    return State::Invalid;
  }
};

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
    setText(QDir::rootPath());
    setValidator(new DirValidator{this});
    CONNECT(this, textEdited, this, updateText);
  }

private:
  QString baseDir;
  QString filterName;
  QStringList entries;
  int beginIndex = -1;
  int endIndex = -1;
  int index = -1;
  bool shown = false;

  bool splitPath(const QString &path) {
    const int slash = path.lastIndexOf(QDir::separator());
    QString newBaseDir;
    if (slash == -1) {
      newBaseDir = QDir::rootPath();
      filterName = path;
    } else {
      newBaseDir = path.left(slash + 1);
      filterName = path.right(path.size() - slash - 1);
    }
    if (newBaseDir != baseDir) {
      baseDir = newBaseDir;
      return true;
    } else {
      return false;
    }
  }
  
  void updateEntries() {
    const QDir::SortFlags sort = QDir::Name;
    const QDir::Filters filter = QDir::Dirs | QDir::Drives | QDir::NoDotAndDotDot;
    entries = QDir{baseDir, {}, sort, filter}.entryList();
  }
  
  int findFirstMatch() const {
    // TODO: binary search
    // Also consider using QDir::IgnoreCase
    for (int i = 0; i != entries.size(); ++i) {
      if (entries[i].startsWith(filterName)) {
        return i;
      }
    }
    return -1;
  }
  
  void updateFilteredRange() {
    if (filterName.isEmpty()) {
      beginIndex = 0;
      endIndex = entries.size();
      return;
    }
    beginIndex = findFirstMatch();
    if (beginIndex == -1) {
      endIndex = -1;
    } else {
      endIndex = beginIndex + 1;
      while (endIndex < entries.size() && entries[endIndex].startsWith(filterName)) {
        ++endIndex;
      }
    }
  }
  
  void resetSuggestion() {
    if (splitPath(text())) {
      updateEntries();
      index = findFirstMatch();
    }
    filterName.clear();
    updateFilteredRange();
    setText(baseDir);
  }

  void cycle(const int dir) {
    if (shown) {
      if (beginIndex == -1) {
        index = -1;
      } else {
        index = std::clamp(index + dir, beginIndex, endIndex - 1);
      }
      suggest();
    } else if (!entries.isEmpty()) {
      shown = true;
      resetSuggestion();
      suggest();
    }
  }
  
  void suggest() {
    if (entries.isEmpty()) return;
    if (beginIndex == -1) return;
    if (index == -1) index = beginIndex;
    
    setText(baseDir + entries[index]);
    setSelection(
      baseDir.size() + filterName.size(),
      entries[index].size() - filterName.size()
    );
  }
  
  void updateText(const QString &path) {
    if (!shown) return;
    if (splitPath(path)) {
      updateEntries();
      updateFilteredRange();
      index = beginIndex;
    } else {
      updateFilteredRange();
      if (index != -1 && !entries[index].startsWith(filterName)) {
        index = beginIndex;
      }
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
        cycle(-1);
        break;
      case Qt::Key_Down:
        cycle(1);
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

void FileInputWidget::setPath(const QString &newDir) {
  text->setText(QDir{newDir}.absolutePath());
  changePath();
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

void FileInputWidget::simplifyPath() {
  text->setText(QDir{path()}.absolutePath());
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
