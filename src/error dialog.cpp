//
//  error dialog.cpp
//  Animera
//
//  Created by Indi Kernick on 17/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
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
  const QString &message
) : Dialog{parent} {
  setWindowTitle("Error");
  setStyleSheet("background-color:" + glob_main.name());
  auto *layout = new QVBoxLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(glob_margin, glob_margin, glob_margin, glob_margin);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->addWidget(makeLabel(this, title));
  // TODO: remove margin from separator
  layout->addWidget(new HoriSeparator{this});
  layout->addWidget(makeWrappedLabel(this, erro_wrap_width, message));
  auto *ok = new TextPushButtonWidget{this, textBoxRect(8), "Ok"};
  CONNECT(ok, pressed, this, accept);
  layout->addWidget(ok, 0, Qt::AlignHCenter);
}
