//
//  timeline layers widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 24/6/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_timeline_layers_widget_hpp
#define animera_timeline_layers_widget_hpp

#include "cell.hpp"
#include <QtWidgets/qscrollarea.h>

class VisibleWidget;
class TextInputWidget;
class QVBoxLayout;

class LayerNameWidget final : public QWidget {
  Q_OBJECT

public:
  LayerNameWidget(QWidget *, LayerIdx);
  
Q_SIGNALS:
  void visibilityChanged(LayerIdx, bool);
  void visibilityIsolated(LayerIdx);
  void nameChanged(LayerIdx, std::string_view);

public Q_SLOTS:
  void setVisibility(bool);
  void setName(std::string_view);

private Q_SLOTS:
  void changeVisibility(bool);
  void isolate();
  void changeName(const QString &);

private:
  VisibleWidget *visible = nullptr;
  TextInputWidget *name = nullptr;
  LayerIdx idx;
  
  void createWidgets();
  void setupLayout();
};

class LayersWidget final : public QWidget {
  Q_OBJECT

public:
  explicit LayersWidget(QWidget *);
  
Q_SIGNALS:
  void visibilityChanged(LayerIdx, bool);
  void visibilityIsolated(LayerIdx);
  void nameChanged(LayerIdx, std::string_view);

public Q_SLOTS:
  void setMargin(int);
  void setVisibility(LayerIdx, bool);
  void setName(LayerIdx, std::string_view);
  void setLayerCount(LayerIdx);
  
private:
  QVBoxLayout *layout = nullptr;
  std::vector<LayerNameWidget *> layers;
  
  LayerIdx layerCount() const;
};

class LayerScrollWidget final : public QScrollArea {
  Q_OBJECT
  
public:
  explicit LayerScrollWidget(QWidget *);

  LayersWidget *getChild();

Q_SIGNALS:
  void shouldSetBottomMargin(int);

private:
  void paintEvent(QPaintEvent *) override;
};

#endif
