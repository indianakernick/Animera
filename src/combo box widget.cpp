//
//  combo box widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "combo box widget.hpp"

#include "connect.hpp"
#include <QtGui/qevent.h>
#include "global font.hpp"
#include <QtGui/qpainter.h>
#include "widget painting.hpp"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qabstractbutton.h>

namespace {

constexpr int item_height = glob_font_px + 2 * glob_text_margin;

class PopupList final : public QWidget {
public:
  PopupList(QWidget *parent, const QSize size)
    : QWidget{parent} {
    setFixedSize(size);
  }

private:
  QRect innerRect() {
    constexpr int bord = glob_border_width;
    return {
      rect().x() + bord, rect().y() + bord,
      rect().width() - 2 * bord, rect().height() - 2 * bord
    };
  }

  void paintEvent(QPaintEvent *) override {
    // @TODO this is lazy and inefficient
    QPainter painter{this};
    painter.fillRect(rect(), glob_border_color);
    painter.fillRect(innerRect(), glob_main);
  }
};

class PopupItem final : public QAbstractButton {
public:
  PopupItem(QWidget *parent, const QString &text)
    : QAbstractButton{parent}, text{text} {}

private:
  QString text;
  bool hover = false;
  
  void enterEvent(QEvent *) override {
    hover = true;
    repaint();
  }
  void leaveEvent(QEvent *) override {
    hover = false;
    repaint();
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    if (hover) {
      painter.fillRect(rect(), glob_light_1);
    }
    painter.setFont(getGlobalFont());
    painter.setBrush(Qt::NoBrush);
    painter.setPen(glob_text_color);
    painter.drawText(
      QPoint{glob_text_margin, glob_text_margin + glob_font_accent_px},
      text
    );
  }
};

}

class ComboBoxPopup final : public QMainWindow {
public:
  explicit ComboBoxPopup(ComboBoxWidget *box)
    : QMainWindow{box}, box{box} {
    setWindowFlag(Qt::FramelessWindowHint);
  }
  
  void init(const QRect outer) {
    setFixedSize(outer.width(), getHeight());
    QPoint pos = box->mapToGlobal(outer.topLeft());
    pos.ry() -= box->currentIndex() * item_height;
    move(pos);
    auto *list = new PopupList{this, {outer.width(), getHeight()}};
    setCentralWidget(list);
    // @TODO pass in WidgetRect
    const int innerWidth = outer.width() - 2 * glob_border_width;
    populateList(list, innerWidth);
    setFocus();
    show();
  }
  
  void quit() {
    hide();
  }

private:
  ComboBoxWidget *box;
  
  int getHeight() const {
    constexpr int border_height = 2 * glob_border_width;
    return border_height + box->count() * item_height;
  }
  
  void populateList(PopupList *list, const int width) {
    auto *layout = new QVBoxLayout{list};
    layout->setContentsMargins(glob_border_width, glob_border_width, glob_border_width, glob_border_width);
    layout->setSpacing(0);
    
    const int count = box->count();
    for (int i = 0; i != count; ++i) {
      auto *item = new PopupItem{list, box->itemText(i)};
      item->setFixedSize(width, item_height);
      CONNECT_LAMBDA(item, pressed, [this, i]{
        box->setCurrentIndex(i);
        quit();
      });
      layout->addWidget(item);
    }
  }
  
  void focusOutEvent(QFocusEvent *) override {
    quit();
  }
};

ComboBoxWidget::ComboBoxWidget(QWidget *parent, const int chars)
  : QWidget{parent}, rects{textBoxIconRect(chars)} {
  popup = new ComboBoxPopup{this};
  setCursor(Qt::PointingHandCursor);
  setFont(getGlobalFont());
  setFixedSize(rects.widget().size());
  arrow = bakeColoredBitmap(":/General/up down arrow.pbm", glob_light_2);
}

void ComboBoxWidget::clear() {
  items.clear();
  setCurrentIndex(-1);
}

void ComboBoxWidget::addItem(const QString &text) {
  items.push_back(text);
  if (items.size() == 1) {
    setCurrentIndex(0);
  }
  update();
}

void ComboBoxWidget::setCurrentIndex(const int index) {
  current = index;
  Q_EMIT currentIndexChanged(index);
  update();
}

int ComboBoxWidget::count() const {
  return static_cast<int>(items.size());
}

QString ComboBoxWidget::itemText(const int index) const {
  assert(0 <= index && index < static_cast<int>(items.size()));
  return items[index];
}

int ComboBoxWidget::currentIndex() const {
  return current;
}

QString ComboBoxWidget::currentText() const {
  assert(0 <= current && current < static_cast<int>(items.size()));
  return items[current];
}

void ComboBoxWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    popup->init(rects.outer());
  }
}

void ComboBoxWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  paintBorder(painter, rects.text(), glob_border_color);
  painter.fillRect(rects.textInner(), glob_dark_1);
  painter.fillRect(rects.iconInner(), glob_main);
  painter.drawPixmap(rects.iconPos(), arrow);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  painter.setFont(getGlobalFont());
  painter.setClipRect(rects.textInner());
  QPoint textPos = rects.textPos();
  textPos.ry() += glob_font_accent_px;
  painter.drawText(textPos, currentText());
}

#include "combo box widget.moc"
