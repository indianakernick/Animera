//
//  editor widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "editor widget.hpp"

#include "config.hpp"
#include "connect.hpp"
#include "composite.hpp"
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qlabel.h>
#include "surface factory.hpp"
#include <QtWidgets/qscrollbar.h>

class EditorScrollBar final : public QScrollBar {
public:
  EditorScrollBar(Qt::Orientation orient, QWidget *parent)
    : QScrollBar{orient, parent} {
    if (orient == Qt::Vertical) {
      setStyleSheet("width: " + QString::number(edit_scroll_width));
    } else if (orient == Qt::Horizontal) {
      setStyleSheet("height: " + QString::number(edit_scroll_width));
    } else {
      Q_UNREACHABLE();
    }
  }
  
private:
  int pagePixels(const int length) const {
    return (length * pageStep()) / (maximum() - minimum() + pageStep());
  }
  int valuePixels(const int length) const {
    if (minimum() == maximum()) {
      return 0;
    } else {
      return (length - pagePixels(length)) * value() / (maximum() - minimum());
    }
  }

  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.fillRect(rect(), edit_scroll_back);
    if (orientation() == Qt::Horizontal) {
      painter.fillRect(QRect{
        valuePixels(width()),
        0,
        pagePixels(width()),
        height()
      }, edit_scroll_handle);
    } else if (orientation() == Qt::Vertical) {
      painter.fillRect(QRect{
        0,
        valuePixels(height()),
        width(),
        pagePixels(height())
      }, edit_scroll_handle);
    } else {
      Q_UNREACHABLE();
    }
  }
};

class EditorCorner final : public QWidget {
public:
  explicit EditorCorner(QWidget *parent)
    : QWidget{parent} {}
  
private:
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.fillRect(rect(), edit_scroll_corner);
  }
};

class EditorImage final : public QWidget {
  Q_OBJECT
  
public:
  explicit EditorImage(QScrollArea *parent)
    : QWidget{parent}, parent{parent} {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    CONNECT(parent->horizontalScrollBar(), actionTriggered, this, updateMouse);
    CONNECT(parent->verticalScrollBar(),   actionTriggered, this, updateMouse);
  }
  
  void setImage(const QImage &img) {
    if (editor.size() != img.size()) {
      resize(img.size());
    }
    editor = img;
    repaint();
  }
  
  void zoomIn() {
    zoom(1);
  }
  void zoomOut() {
    zoom(-1);
  }
  void resize() {
    updateCheckers();
  }
  
Q_SIGNALS:
  void mouseLeave(QImage *);
  void mouseDown(QPoint, ButtonType, QImage *);
  void mouseMove(QPoint, QImage *);
  void mouseUp(QPoint, ButtonType, QImage *);
  void keyPress(Qt::Key, QImage *);
  
private:
  QScrollArea *parent;
  QImage checkers;
  QImage overlay;
  QImage editor;
  QPoint pos;
  int scale = edit_default_scale;
  int keysDown = 0;

  void zoom(const int dir) {
    const int oldScale = scale;
    scale = std::clamp(scale + dir, edit_min_scale, edit_max_scale);
    if (scale == oldScale) return;
    setFixedSize(editor.size() * scale);
    adjustScroll(oldScale);
    updateMouse();
    updateCheckers();
    repaint();
  }

  void adjustScroll(const int oldScale) {
    QScrollBar *hbar = parent->horizontalScrollBar();
    QScrollBar *vbar = parent->verticalScrollBar();
    if (width() >= parent->width()) {
      const int halfWidth = parent->width() / 2;
      hbar->setValue((hbar->value() + halfWidth) * scale / oldScale - halfWidth);
    }
    if (height() >= parent->height()) {
      const int halfHeight = parent->height() / 2;
      vbar->setValue((vbar->value() + halfHeight) * scale / oldScale - halfHeight);
    }
  }
  
  void updateMouse() {
    pos = getPos();
    Q_EMIT mouseMove(pos, &overlay);
  }

  void resize(const QSize newSize) {
    overlay = QImage{newSize, QImage::Format_ARGB32};
    clearImage(overlay);
    setFixedSize(newSize * scale);
    updateCheckers();
  }

  void updateCheckers() {
    QSize size = rect().intersected(parent->viewport()->rect()).size();
    size.setWidth(size.width() - size.width() % scale + 4 * scale);
    size.setHeight(size.height() - size.height() % scale + 4 * scale);
    updateCheckers(size);
  }
  
  void updateCheckers(const QSize size) {
    /*
    
    scale = 3
    
    O O * O * *
    O O * O * *
    * * O * O O
    O O * O * *
    * * O * O O
    * * O * O O
    
    */
  
    checkers = QImage{size, QImage::Format_ARGB32};
    const int lilScale = scale / 2;
    const int bigScale = lilScale + scale % 2;
    Surface surface = makeSurface<QRgb>(checkers);
    int x = 0;
    int y = 0;
    for (auto row : surface.range()) {
      for (QRgb &pixel : row) {
        const bool color = (x < bigScale || (scale <= x && x < scale + lilScale))
                        == (y < bigScale || (scale <= y && y < scale + lilScale));
        pixel = color ? edit_checker_a : edit_checker_b;
        x = (x + 1) % (scale * 2);
      }
      y = (y + 1) % (scale * 2);
      x = 0;
    }
  }
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    QScrollBar *hbar = parent->horizontalScrollBar();
    QScrollBar *vbar = parent->verticalScrollBar();
    const QPoint viewPos = {hbar->value(), vbar->value()};
    const QSize viewSize = rect().intersected(parent->viewport()->rect()).size();
    painter.drawImage({viewPos, viewSize}, checkers, QRect{{viewPos.x() % (scale * 2), viewPos.y() % (scale * 2)}, viewSize});
    painter.drawImage(rect(), editor);
    painter.drawImage(rect(), overlay);
  }
  
  QPoint getPixelPos(const QPointF localPos) {
    const QPointF posF = localPos / scale;
    return {static_cast<int>(posF.x()), static_cast<int>(posF.y())};
  }
  QPoint getPos(QMouseEvent *event) {
    return getPixelPos(event->localPos());
  }
  QPoint getPos() {
    return getPixelPos(mapFromGlobal(QCursor::pos()));
  }
  ButtonType getButton(QMouseEvent *event) {
    switch (event->button()) {
      case mouse_primary:   return ButtonType::primary;
      case mouse_secondary: return ButtonType::secondary;
      case mouse_tertiary:  return ButtonType::erase;
      default: Q_UNREACHABLE();
    }
  }
  ButtonType getButton(QKeyEvent *event) {
    switch (event->key()) {
      case key_primary:   return ButtonType::primary;
      case key_secondary: return ButtonType::secondary;
      case key_tertiary:  return ButtonType::erase;
      default: return ButtonType::none;
    }
  }

  void mousePressEvent(QMouseEvent *event) override {
    pos = getPos(event);
    Q_EMIT mouseDown(pos, getButton(event), &overlay);
  }
  void mouseReleaseEvent(QMouseEvent *event) override {
    pos = getPos(event);
    Q_EMIT mouseUp(pos, getButton(event), &overlay);
  }
  void mouseMoveEvent(QMouseEvent *event) override {
    pos = getPos(event);
    Q_EMIT mouseMove(pos, &overlay);
  }
  void enterEvent(QEvent *) override {
    setFocus();
  }
  void leaveEvent(QEvent *) override {
    Q_EMIT mouseLeave(&overlay);
  }

public:
  void keyPressEvent(QKeyEvent *event) override {
    if (!event->isAutoRepeat()) {
      ++keysDown;
      if (keysDown == 1) grabMouse(cursor());
    }
    if (event->key() == key_zoom_out) {
      zoomOut();
    } else if (event->key() == key_zoom_in) {
      zoomIn();
    } else if (ButtonType button = getButton(event); button != ButtonType::none) {
      if (!event->isAutoRepeat()) {
        Q_EMIT mouseDown(pos, button, &overlay);
      }
    } else {
      Q_EMIT keyPress(static_cast<Qt::Key>(event->key()), &overlay);
    }
  }
  void keyReleaseEvent(QKeyEvent *event) override {
    --keysDown;
    if (keysDown == 0) releaseMouse();
    const ButtonType button = getButton(event);
    if (button != ButtonType::none) {
      Q_EMIT mouseUp(pos, button, &overlay);
    }
  }
};

EditorWidget::EditorWidget(QWidget *parent, Animation &anim)
  : QScrollArea{parent}, anim{anim}, view{new EditorImage{this}} {
  setAlignment(Qt::AlignCenter);
  setFocusPolicy(Qt::WheelFocus);
  setVerticalScrollBar(new EditorScrollBar{Qt::Vertical, this});
  setHorizontalScrollBar(new EditorScrollBar{Qt::Horizontal, this});
  setCornerWidget(new EditorCorner{this});
  view = new EditorImage{this};
  setWidget(view);
  setFrameShape(NoFrame);
  CONNECT(view, mouseLeave, this, mouseLeave);
  CONNECT(view, mouseDown,  this, mouseDown);
  CONNECT(view, mouseMove,  this, mouseMove);
  CONNECT(view, mouseUp,    this, mouseUp);
  CONNECT(view, keyPress,   this, keyPress);
  setStyleSheet("background-color: " + glob_back_color.name());
}

void EditorWidget::composite() {
  view->setImage(compositeFrame(anim.getPallete(), anim.getFrame(frame), visibility));
}

void EditorWidget::compositeOverlay() {
  view->repaint();
}

void EditorWidget::compositePos(Cell *, LayerIdx newLayer, FrameIdx newFrame) {
  layer = newLayer;
  frame = newFrame;
  composite();
}

void EditorWidget::compositeVis(const LayerVisible &newVisibility) {
  visibility = newVisibility;
  composite();
}

// @TODO Use QAbstractScrollArea and remove this hack

void EditorWidget::enterEvent(QEvent *) {
  view->setFocus();
}

void EditorWidget::leaveEvent(QEvent *) {
  view->clearFocus();
}

void EditorWidget::resizeEvent(QResizeEvent *) {
  view->resize();
}

#include "editor widget.moc"
