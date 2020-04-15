//
//  error dialog.cpp
//  Animera
//
//  Created by Indiana Kernick on 17/8/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "error dialog.hpp"

#include "connect.hpp"
#include "label widget.hpp"
#include "separator widget.hpp"
#include <QtWidgets/qboxlayout.h>
#include "text push button widget.hpp"

ErrorDialog::ErrorDialog(
  QWidget *parent,
  const QString &title,
  QString message
) : Dialog{parent} {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Error");
  setStyleSheet("background-color:" + glob_main.name());
  
  auto *layout = new QVBoxLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  
  const WidgetRect titleRect = textBoxRect(title.size());
  const WidgetRect titleRectMargins = addMargins(titleRect, true, true, true, false);
  layout->addWidget(new LabelWidget{this, titleRectMargins, title});
  layout->addWidget(new HoriSeparator{this});
  
  const QSize bodySize = wrapToWidth(message, erro_wrap_width);
  const WidgetRect bodyRect = textBoxRect(bodySize);
  const WidgetRect bodyRectMargins = addMargins(bodyRect, true, false, true, false);
  layout->addWidget(new LabelWidget{this, bodyRectMargins, message});
  
  constexpr WidgetRect buttonRect = addMargins(textBoxRect(8), true, false, true, true);
  auto *ok = new TextPushButtonWidget{this, buttonRect, "Ok"};
  CONNECT(ok, pressed, this, accept);
  layout->addWidget(ok, 0, Qt::AlignHCenter);
}
