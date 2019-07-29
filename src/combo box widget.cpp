//
//  combo box widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "combo box widget.hpp"

#include "connect.hpp"
#include "global font.hpp"
#include <QtGui/qpainter.h>
#include "widget painting.hpp"
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qabstractbutton.h>

namespace {

constexpr int item_height = glob_font_px + 2 * glob_text_padding;

class PopupList final : public QWidget {
public:
  PopupList(QWidget *parent, const QSize size)
    : QWidget{parent} {
    setFixedSize(size);
  }

private:
  QRect innerRect() {
    constexpr int bord = glob_border_width;
    return {rect().x() + bord, rect().y() + bord, rect().width() - 2 * bord, rect().height() - 2 * bord};
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
  PopupItem(QWidget *parent, const int width, ComboBoxWidget *box, const int idx)
    : QAbstractButton{parent}, box{box},  idx{idx} {
    setFixedSize(width, item_height);
    CONNECT(this, pressed, this, setCurrentIndex);
  }

private Q_SLOTS:
  void setCurrentIndex() {
    box->setCurrentIndex(idx);
    box->hidePopup();
  }

private:
  ComboBoxWidget *box;
  int idx;
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
      QPoint{glob_text_padding, glob_text_padding + glob_font_accent_px},
      box->itemText(idx)
    );
  }
};

}

class ComboBoxPopup final : public QMainWindow {
public:
  ComboBoxPopup(ComboBoxWidget *parent, const QRect outer)
    : QMainWindow{parent}, box{parent} {
    setWindowFlag(Qt::FramelessWindowHint);
    setFixedSize(outer.width(), getHeight());
    QPoint pos = parent->mapToGlobal(outer.topLeft());
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
      layout->addWidget(new PopupItem{list, width, box, i});
    }
  }
  
  void focusOutEvent(QFocusEvent *) override {
    box->hidePopup();
  }
};

ComboBoxWidget::ComboBoxWidget(QWidget *parent, const int chars)
  : QComboBox{parent}, rects{textBoxIconRect(chars)} {
  setCursor(Qt::PointingHandCursor);
  setFont(getGlobalFont());
  setFixedSize(rects.widget().size());
  arrow = bakeColoredBitmap(":/General/up down arrow.pbm", glob_light_2);
}

void ComboBoxWidget::showPopup() {
  QComboBox::hidePopup();
  assert(popup == nullptr);
  popup = new ComboBoxPopup{this, rects.outer()};
}

void ComboBoxWidget::hidePopup() {
  QComboBox::hidePopup();
  delete popup;
  popup = nullptr;
}

void ComboBoxWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  paintBorder(painter, rects.text(), glob_border_color);
  paintBorder(painter, rects.icon(), glob_border_color);
  painter.fillRect(rects.border(), glob_border_color);
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
