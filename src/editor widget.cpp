//
//  editor widget.cpp
//  Animera
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "editor widget.hpp"

#include <cmath>
#include "config.hpp"
#include "connect.hpp"
#include "composite.hpp"
#include <QtGui/qevent.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/qlabel.h>
#include "surface factory.hpp"
#include <QtWidgets/qgesture.h>
#include <QtWidgets/qscrollbar.h>

class EditorImage final : public QWidget {
  Q_OBJECT
  
public:
  explicit EditorImage(QScrollArea *parent)
    : QWidget{parent}, parent{parent} {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    CONNECT(parent->horizontalScrollBar(), actionTriggered, this, updateMouse);
    CONNECT(parent->verticalScrollBar(),   actionTriggered, this, updateMouse);
  }
  
  void setImage(const QImage &img) {
    editor = img;
    repaint();
  }
  
  void setSize(const QSize size) {
    overlay = QImage{size, QImage::Format_ARGB32};
    clearImage(overlay);
    const QSize viewSize = parent->viewport()->rect().size();
    scale = std::min(viewSize.width() / size.width(), viewSize.height() / size.height());
    scale = std::clamp(scale, edit_min_scale, edit_max_scale);
    setFixedSize(size * scale);
    updateCheckers();
    Q_EMIT resized();
  }
  
  void resize() {
    updateCheckers();
  }
  
  QImage *getOverlay() {
    return &overlay;
  }
  
  void setScale(const int newScale) {
    const int oldScale = scale;
    scale = std::clamp(newScale, edit_min_scale, edit_max_scale);
    if (scale == oldScale) return;
    zoomIntoCenter(oldScale);
    updateMouse();
    updateCheckers();
    Q_EMIT resized();
    repaint();
  }
  int getScale() const {
    return scale;
  }
  
  void zoom(const int dir) {
    setScale(scale + dir);
  }
  
Q_SIGNALS:
  void mouseLeave();
  void mouseDown(QPoint, ButtonType);
  void mouseMove(QPoint);
  void mouseUp(QPoint, ButtonType);
  void keyPress(Qt::Key);
  void resized();
  
private:
  QScrollArea *parent;
  QImage checkers;
  QImage overlay;
  QImage editor;
  QPoint pos;
  int scale = edit_min_scale;
  ButtonType buttonDown = ButtonType::none;
  bool keyButton = false;

  void zoomIntoCenter(const int oldScale) {
    // @TODO this could still be improved
    const QSize viewportSize = parent->viewport()->size();
    const QPoint center = mapFromParent(toPoint(viewportSize / 2));
    setFixedSize(editor.size() * scale);
    const QPoint newCenter = (center * scale) / oldScale;
    parent->ensureVisible(newCenter.x(), newCenter.y(), viewportSize.width() / 2, viewportSize.height() / 2);
  }
  
  void checkMouseLeave() {
    if (!rect().contains(pos)) {
      Q_EMIT mouseLeave();
    }
  }
  
  void updateMouse() {
    pos = getPos();
    Q_EMIT mouseMove(pos);
    checkMouseLeave();
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
    const ButtonType button = getButton(event);
    if (buttonDown == ButtonType::none) {
      buttonDown = button;
      keyButton = false;
      grabMouse(cursor());
      pos = getPos(event);
      Q_EMIT mouseDown(pos, button);
    }
  }
  void mouseReleaseEvent(QMouseEvent *event) override {
    const ButtonType button = getButton(event);
    if (button == buttonDown && !keyButton) {
      buttonDown = ButtonType::none;
      releaseMouse();
      pos = getPos(event);
      Q_EMIT mouseUp(pos, getButton(event));
      checkMouseLeave();
    }
  }
  void mouseMoveEvent(QMouseEvent *event) override {
    pos = getPos(event);
    Q_EMIT mouseMove(pos);
    setCursor(cursor());
  }
  void enterEvent(QEvent *) override {
    setFocus();
  }
  void leaveEvent(QEvent *) override {
    Q_EMIT mouseLeave();
    clearFocus();
  }

public:
  void keyPressEvent(QKeyEvent *event) override {
    if (event->key() == key_zoom_out) {
      zoom(-1);
    } else if (event->key() == key_zoom_in) {
      zoom(1);
    } else if (ButtonType button = getButton(event); button != ButtonType::none) {
      if (!event->isAutoRepeat()) {
        if (buttonDown == ButtonType::none) {
          buttonDown = button;
          keyButton = true;
          grabMouse(cursor());
          grabKeyboard();
          Q_EMIT mouseDown(pos, button);
        }
      }
    } else {
      Q_EMIT keyPress(static_cast<Qt::Key>(event->key()));
    }
  }
  void keyReleaseEvent(QKeyEvent *event) override {
    const ButtonType button = getButton(event);
    if (button != ButtonType::none) {
      if (button == buttonDown && keyButton) {
        buttonDown = ButtonType::none;
        releaseKeyboard();
        releaseMouse();
        Q_EMIT mouseUp(pos, button);
        checkMouseLeave();
      }
    }
  }
};

EditorWidget::EditorWidget(QWidget *parent)
  : ScrollAreaWidget{parent} {
  setFocusPolicy(Qt::NoFocus);
  setFrameShape(NoFrame);
  setAlignment(Qt::AlignCenter);
  grabGesture(Qt::PinchGesture);
  view = new EditorImage{this};
  setWidget(view);
  CONNECT(view, mouseLeave, this, mouseLeave);
  CONNECT(view, mouseDown,  this, mouseDown);
  CONNECT(view, mouseMove,  this, mouseMove);
  CONNECT(view, mouseUp,    this, mouseUp);
  CONNECT(view, keyPress,   this, keyPress);
  CONNECT(view, resized,    this, adjustMargins);
  setStyleSheet("background-color: " + glob_back_color.name());
}

void EditorWidget::composite() {
  view->setImage(compositeFrame(palette, frame, size, format));
}

void EditorWidget::compositeOverlay() {
  view->repaint();
}

void EditorWidget::compositePalette() {
  if (format == Format::index) {
    composite();
  }
}

void EditorWidget::setFrame(const Frame &newFrame) {
  frame = newFrame;
  composite();
}

void EditorWidget::setPalette(const PaletteCSpan newPalette) {
  palette = newPalette;
}

void EditorWidget::initCanvas(const Format newFormat, const QSize newSize) {
  format = newFormat;
  size = newSize;
  view->setSize(newSize);
  Q_EMIT overlayChanged(view->getOverlay());
}

bool EditorWidget::event(QEvent *event) {
  if (event->type() == QEvent::Gesture) {
    auto *gstEvent = static_cast<QGestureEvent *>(event);
    QGesture *gesture = gstEvent->gesture(Qt::PinchGesture);
    auto *pinch = static_cast<QPinchGesture *>(gesture);
    switch (pinch->state()) {
      case Qt::GestureStarted:
        startScale = view->getScale();
        break;
      case Qt::GestureUpdated:
      case Qt::GestureFinished:
      case Qt::GestureCanceled:
        if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged) {
          view->setScale(qRound(startScale * pinch->totalScaleFactor()));
        }
        break;
      case Qt::NoGesture: ;
    }
    return true;
  } else {
    return QScrollArea::event(event);
  }
}

void EditorWidget::resizeEvent(QResizeEvent *event) {
  view->resize();
  adjustMargins();
  QScrollArea::resizeEvent(event);
}

#include "editor widget.moc"
