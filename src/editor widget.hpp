//
//  editor widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef editor_widget_hpp
#define editor_widget_hpp

#include "tool.hpp"
#include "animation.hpp"
#include "scroll bar widget.hpp"

class EditorImage;

class EditorWidget final : public ScrollAreaWidget {
  Q_OBJECT

public:
  explicit EditorWidget(QWidget *);

Q_SIGNALS:
  void mouseLeave(QImage *);
  void mouseDown(QPoint, ButtonType, QImage *);
  void mouseMove(QPoint, QImage *);
  void mouseUp(QPoint, ButtonType, QImage *);
  void keyPress(Qt::Key, QImage *);

public Q_SLOTS:
  void composite();
  void compositeOverlay();
  void compositePalette();
  void changeFrame(const Frame &);
  void changeVisible(const LayerVisible &);
  void changePalette(const Palette *);
  void initCanvas(Format, QSize);

private:
  EditorImage *view;
  LayerVisible visibility;
  Frame frame;
  const Palette *palette = nullptr;
  QSize size;
  Format format;

  void resizeEvent(QResizeEvent *) override;
};

#endif
