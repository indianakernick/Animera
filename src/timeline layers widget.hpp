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
#include "icon radio button widget.hpp"

class TextInputWidget;
class QVBoxLayout;

class LayerNameWidget final : public QWidget {
  Q_OBJECT

public:
  LayerNameWidget(QWidget *, LayerIdx);
  
Q_SIGNALS:
  void visibilityChanged(LayerIdx, bool);
  void nameChanged(LayerIdx, std::string_view);

public Q_SLOTS:
  void setVisibility(bool);
  void setName(std::string_view);

private:
  IconRadioButtonWidget *visible = nullptr;
  TextInputWidget *name = nullptr;
  LayerIdx idx;
  
  void paintBack(QPixmap &);
  void paintIcon(QPixmap &, const QString &);
  void createWidgets();
  void setupLayout();
  
  void paintEvent(QPaintEvent *) override;
};

class LayersWidget final : public QWidget {
  Q_OBJECT

public:
  explicit LayersWidget(QWidget *);
  
Q_SIGNALS:
  void visibilityChanged(LayerIdx, bool);
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
