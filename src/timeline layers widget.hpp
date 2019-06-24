//
//  timeline layers widget.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 24/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef timeline_layers_widget_hpp
#define timeline_layers_widget_hpp

#include "cell.hpp"
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/qabstractbutton.h>

class TextInputWidget;
class QVBoxLayout;

class VisibleWidget final : public QAbstractButton {
public:
  explicit VisibleWidget(QWidget *);

private:
  QPixmap shownPix;
  QPixmap hiddenPix;

  void loadIcons();

  void paintEvent(QPaintEvent *) override;
};

class LayerNameWidget final : public QWidget {
public:
  // @TODO serialize name and visibility

  LayerNameWidget(QWidget *, LayerIdx);
  
  bool getVisible() const;
  QString getName() const;

private:
  VisibleWidget *visible = nullptr;
  TextInputWidget *name = nullptr;
  
  void paintEvent(QPaintEvent *) override;
  
  void setupLayout();
};

class LayersWidget final : public QWidget {
public:
  explicit LayersWidget(QWidget *);
  
  void appendLayer(LayerIdx);
  
private:
  QVBoxLayout *layout = nullptr;
};

class LayerScrollWidget final : public QScrollArea {
  Q_OBJECT
  
public:
  explicit LayerScrollWidget(QWidget *);

public Q_SLOTS:
  void changeBottomMargin(int);

private:
  void paintEvent(QPaintEvent *) override;
};

#endif
