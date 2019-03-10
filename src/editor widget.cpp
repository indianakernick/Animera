//
//  editor widget.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "editor widget.hpp"

#include <QtWidgets/qlabel.h>

class EditorImage final : public QLabel {
  Q_OBJECT
  
public:
  explicit EditorImage(QWidget *parent)
    : QLabel{parent} {}
  
Q_SIGNALS:
  void mouseDown(QPoint, Qt::MouseButton, QImage *);
  void mouseMove(QPoint, QImage *);
  void mouseUp(QPoint, Qt::MouseButton, QImage *);
  void keyPress(Qt::Key, QImage *);
  
private:
  void mousePressEvent(QMouseEvent *) override {};
  void mouseReleaseEvent(QMouseEvent *) override {};
  void mouseMoveEvent(QMouseEvent *) override {};
  void keyPressEvent(QKeyEvent *) override {};
  void keyReleaseEvent(QKeyEvent *) override {};
};

EditorWidget::EditorWidget(QWidget *parent, Animation &anim)
  : QScrollArea{parent}, anim{anim}, view{new EditorImage{this}} {
  setWidget(view);
  connect(view, &EditorImage::mouseDown, this, &EditorWidget::mouseDown);
  connect(view, &EditorImage::mouseMove, this, &EditorWidget::mouseMove);
  connect(view, &EditorImage::mouseUp, this, &EditorWidget::mouseUp);
  connect(view, &EditorImage::keyPress, this, &EditorWidget::keyPress);
}

void EditorWidget::composite() {
  
}

void EditorWidget::composite(Cell *, LayerIdx newLayer, FrameIdx newFrame) {
  layer = newLayer;
  frame = newFrame;
  composite();
}

void EditorWidget::composite(const LayerVisible &newVisibility) {
  visibility = newVisibility;
  composite();
}

void EditorWidget::keyPressEvent(QKeyEvent *) {}
void EditorWidget::keyReleaseEvent(QKeyEvent *) {}

#include "editor widget.moc"
