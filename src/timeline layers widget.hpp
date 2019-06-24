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
  Q_OBJECT

public:
  // @TODO serialize name and visibility

  LayerNameWidget(QWidget *, LayerIdx);
  
  bool getVisible() const;
  QString getName() const;

Q_SIGNALS:
  void visibleToggled();

private:
  VisibleWidget *visible = nullptr;
  TextInputWidget *name = nullptr;
  
  void paintEvent(QPaintEvent *) override;
  
  void setupLayout();
};

class LayersWidget final : public QWidget {
  Q_OBJECT

public:
  explicit LayersWidget(QWidget *);
  
  void appendLayer(LayerIdx);

Q_SIGNALS:
  void visibleChanged(const LayerVisible &);

private Q_SLOTS:
  void toggleVisible();

public Q_SLOTS:
  void setMargin(int);
  
private:
  QVBoxLayout *layout = nullptr;
  std::vector<LayerNameWidget *> layers;
};

class LayerScrollWidget final : public QScrollArea {
  Q_OBJECT
  
public:
  explicit LayerScrollWidget(QWidget *);

  LayersWidget *setChild(LayersWidget *);

Q_SIGNALS:
  void changeBottomMargin(int);

private:
  void paintEvent(QPaintEvent *) override;
};

#endif
