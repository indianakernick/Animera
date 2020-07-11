//
//  timeline layers widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 24/6/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_timeline_layers_widget_hpp
#define animera_timeline_layers_widget_hpp

#include "cel.hpp"
#include <QtWidgets/qscrollarea.h>

class VisibleWidget;
class TextInputWidget;
class QVBoxLayout;

class LayerNameWidget final : public QWidget {
  Q_OBJECT

public:
  LayerNameWidget(QWidget *, LayerIdx);
  
Q_SIGNALS:
  void shouldSetVisibility(LayerIdx, bool);
  void shouldIsolateVisibility(LayerIdx);
  void shouldSetName(LayerIdx, std::string_view);

public Q_SLOTS:
  void setVisibility(bool);
  void setName(std::string_view);

private:
  VisibleWidget *visible = nullptr;
  TextInputWidget *name = nullptr;
  LayerIdx idx;
  
  void createWidgets();
  void setupLayout();
};

class GroupNameWidget final : public QWidget {
  Q_OBJECT

public:
  explicit GroupNameWidget(QWidget *);
  
Q_SIGNALS:
  void shouldSetName(std::string_view);

public Q_SLOTS:
  void setName(std::string_view);

private:
  TextInputWidget *name = nullptr;
};

class LayersWidget final : public QWidget {
  Q_OBJECT

public:
  explicit LayersWidget(QWidget *);
  
Q_SIGNALS:
  void shouldSetVisibility(LayerIdx, bool);
  void shouldIsolateVisibility(LayerIdx);
  void shouldSetName(LayerIdx, std::string_view);

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
