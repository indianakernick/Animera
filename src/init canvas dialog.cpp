//
//  init canvas dialog.cpp
//  Animera
//
//  Created by Indi Kernick on 3/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "init canvas dialog.hpp"

#include "connect.hpp"
#include <QtGui/qevent.h>
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include "label widget.hpp"
#include "color input widget.hpp"
#include "radio button widget.hpp"
#include <QtWidgets/qgridlayout.h>
#include "text push button widget.hpp"

class FormatWidget final : public RadioButtonWidget {
  Q_OBJECT
  
public:
  FormatWidget(QWidget *parent, const QString &name, const Format format)
    : RadioButtonWidget{parent}, icon{name}, fmt{format} {
    setFixedSize(tool_button_size);
    back = QBitmap{":/Tools/base.pbm"}.scaled(tool_icon_size);
  }

  Format format() const {
    return fmt;
  }

private:
  QPixmap icon;
  QBitmap back;
  Format fmt;
  
  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    QRegion region = back;
    region.translate(tool_icon_pos);
    painter.setClipRegion(region);
    painter.setBrush(isChecked() ? tool_base_enabled : tool_base_disabled);
    painter.setPen(Qt::NoPen);
    painter.drawRect(rect());
    painter.drawPixmap({tool_icon_pos, tool_icon_size}, icon);
  }
};

InitCanvasDialog::InitCanvasDialog(QWidget *parent)
  : Dialog{parent} {
  setWindowTitle("New File");
  setStyleSheet("background-color:" + glob_main.name());
  createWidgets();
  setupLayout();
  connectSignals();
}

void InitCanvasDialog::submit() {
  constexpr Format null_format = static_cast<Format>(-1);
  Format format = null_format;
  for (FormatWidget *widget : formats) {
    if (widget->isChecked()) {
      format = widget->format();
      break;
    }
  }
  assert(format != null_format);
  const QSize size = {width->value(), height->value()};
  Q_EMIT canvasInitialized(format, size);
}

void InitCanvasDialog::createWidgets() {
  width = new NumberInputWidget{this, init_size_rect, init_size_range};
  height = new NumberInputWidget{this, init_size_rect, init_size_range};
  formats.push_back(new FormatWidget{this, ":/Formats/rgba.png", Format::rgba});
  formats.push_back(new FormatWidget{this, ":/Formats/gray.png", Format::gray});
  formats.push_back(new FormatWidget{this, ":/Formats/index.png", Format::index});
  formats[0]->setToolTip("32-bit RGBA");
  formats[1]->setToolTip("16-bit Gray-Alpha");
  formats[2]->setToolTip("8-bit Indexed");
  formats.front()->setChecked(true);
  ok = new TextPushButtonWidget{this, init_button_rect, "Create"};
  cancel = new TextPushButtonWidget{this, init_button_rect, "Cancel"};
}

void InitCanvasDialog::setupLayout() {
  auto *layout = new QGridLayout{this};
  layout->setSpacing(0);
  layout->setContentsMargins(glob_margin, glob_margin, glob_margin, glob_margin);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->addWidget(makeLabel(this, 8, "Width: "), 0, 0, Qt::AlignLeft);
  layout->addWidget(width, 0, 1, Qt::AlignRight);
  layout->addWidget(makeLabel(this, 8, "Height: "), 1, 0, Qt::AlignLeft);
  layout->addWidget(height, 1, 1, Qt::AlignRight);
  layout->addWidget(makeLabel(this, 8, "Format: "), 2, 0, Qt::AlignLeft);
  
  auto *formatLayout = new QHBoxLayout{};
  layout->addLayout(formatLayout, 3, 0, 1, 2, Qt::AlignCenter);
  formatLayout->setContentsMargins(0, 0, 0, 0);
  formatLayout->setSpacing(0);
  formatLayout->addStretch();
  for (FormatWidget *widget : formats) {
    formatLayout->addWidget(widget);
    formatLayout->addStretch();
  }
  
  layout->addWidget(ok, 4, 0);
  layout->addWidget(cancel, 4, 1);
}

void InitCanvasDialog::connectSignals() {
  CONNECT(ok,     pressed,  this, accept);
  CONNECT(cancel, pressed,  this, reject);
  CONNECT(this,   accepted, this, submit);
}

#include "init canvas dialog.moc"
