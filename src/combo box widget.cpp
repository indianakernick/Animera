//
//  combo box widget.cpp
//  Animera
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

class PopupList final : public QWidget {
public:
  PopupList(QWidget *parent, const WidgetRect rect)
    : QWidget{parent}, rect{rect} {
    setFixedSize(rect.widget().size());
  }

private:
  WidgetRect rect;
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    paintBorder(painter, rect, glob_border_color);
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
    update();
  }
  void leaveEvent(QEvent *) override {
    // Yes, I actually have to do this.
    // Can't use underMouse.
    // Bug?
    if (!rect().contains(mapFromGlobal(QCursor::pos()))) {
      hover = false;
      update();
    }
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
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
  }
  
  void init(const WidgetRect rect) {
    const WidgetRect adjusted = adjustRect(rect);
    setFixedSize(adjusted.widget().size());
    QPoint pos = box->mapToGlobal(rect.widget().topLeft());
    pos.ry() -= box->currentIndex() * rect.inner().height();
    move(pos);
    auto *list = new PopupList{this, adjusted};
    setCentralWidget(list);
    populateList(list, rect.inner().size());
    setFocus();
    show();
  }
  
  void quit() {
    hide();
  }

private:
  ComboBoxWidget *box;
  
  static QRect addHeight(QRect rect, const int extraHeight) {
    rect.setBottom(rect.bottom() + extraHeight);
    return rect;
  }
  
  WidgetRect adjustRect(const WidgetRect rect) const {
    const QPoint topLeft = rect.widget().topLeft();
    const int extraHeight = rect.inner().height() * (box->count() - 1);
    return {
      addHeight(rect.widget().translated(-topLeft), extraHeight),
      addHeight(rect.outer().translated(-topLeft), extraHeight),
      addHeight(rect.inner().translated(-topLeft), extraHeight),
      rect.pos()
    };
  }
  
  void populateList(PopupList *list, const QSize innerSize) {
    auto *layout = new QVBoxLayout{list};
    layout->setContentsMargins(glob_border_width, glob_border_width, glob_border_width, glob_border_width);
    layout->setSpacing(0);
    
    const int count = box->count();
    for (int i = 0; i != count; ++i) {
      auto *item = new PopupItem{list, box->itemText(i)};
      item->setFixedSize(innerSize);
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
  setFixedSize(rects.text.widget().size() + QSize{rects.icon.widget().width(), 0});
  const int dx = rects.text.widget().width();
  rects.icon = {
    rects.icon.widget().translated(dx, 0),
    rects.icon.outer().translated(dx, 0),
    rects.icon.inner().translated(dx, 0),
    rects.icon.pos() + QPoint{dx, 0}
  };
  outer = rects.text.outer();
  outer.setWidth(outer.width() + rects.icon.outer().width());
  setMask(QRegion{outer});
  arrow = bakeColoredBitmap(":/General/up down arrow.png", glob_light_2);
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
    constexpr int bord = glob_border_width;
    const QRect inner = outer.adjusted(bord, bord, -bord, -bord);
    popup->init({outer, outer, inner, inner.topLeft() + toPoint(glob_text_margin)});
  }
}

void ComboBoxWidget::paintEvent(QPaintEvent *) {
  QPainter painter{this};
  paintBorder(painter, rects.text, glob_border_color);
  paintBorder(painter, rects.icon, glob_border_color);
  painter.fillRect(rects.text.inner(), glob_dark_1);
  painter.fillRect(rects.icon.inner(), glob_main);
  painter.drawPixmap(rects.icon.pos(), arrow);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(glob_text_color);
  painter.setFont(getGlobalFont());
  painter.setClipRect(rects.text.inner());
  QPoint textPos = rects.text.pos();
  textPos.ry() += glob_font_accent_px;
  painter.drawText(textPos, currentText());
}

#include "combo box widget.moc"
