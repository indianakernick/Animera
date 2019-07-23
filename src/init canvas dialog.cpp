//
//  init canvas dialog.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "init canvas dialog.hpp"

#include "connect.hpp"
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
  FormatWidget(QWidget *parent, const QString &name, Format format)
    : RadioButtonWidget{parent}, icon{name}, format{format} {
    setFixedSize(tool_button_size);
    back = QBitmap{":/Tools/base.pbm"}.scaled(tool_icon_size);
    CONNECT(this, toggled, this, check);
  }

Q_SIGNALS:
  void checked(Format);

private Q_SLOTS:
  void check() {
    if (isChecked()) {
      Q_EMIT checked(format);
    }
  }

private:
  QPixmap icon;
  QBitmap back;
  Format format;
  
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

InitCanvasDialog::InitCanvasDialog(QWidget *widget)
  : QDialog{widget} {
  setWindowTitle("New File");
  setStyleSheet("background-color:" + glob_main.name());
  createWidgets();
  setupLayout();
  connectSignals();
}

void InitCanvasDialog::widthChanged(const int width) {
  size.setWidth(width);
}

void InitCanvasDialog::heightChanged(const int height) {
  size.setHeight(height);
}

void InitCanvasDialog::formatChanged(const Format format) {
  colorFormat = format;
}

void InitCanvasDialog::finalize() {
  Q_EMIT canvasInitialized(colorFormat, size);
}

void InitCanvasDialog::createWidgets() {
  widthWidget = new NumberInputWidget{this, init_size_rect, init_size_range};
  heightWidget = new NumberInputWidget{this, init_size_rect, init_size_range};
  formatWidgets.push_back(new FormatWidget{this, ":/Formats/rgba.png", Format::rgba});
  formatWidgets.push_back(new FormatWidget{this, ":/Formats/gray.png", Format::gray});
  formatWidgets.push_back(new FormatWidget{this, ":/Formats/index.png", Format::palette});
  formatWidgets[0]->setToolTip("32-bit RGBA");
  formatWidgets[1]->setToolTip("8-bit Grayscale");
  formatWidgets[2]->setToolTip("8-bit Indexed");
  formatWidgets.front()->setChecked(true);
  okButton = new TextPushButtonWidget{this, init_button_rect, "Ok"};
  cancelButton = new TextPushButtonWidget{this, init_button_rect, "Cancel"};
}

void InitCanvasDialog::setupLayout() {
  auto *layout = new QGridLayout{this};
  setLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(glob_padding, glob_padding, glob_padding, glob_padding);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  QWidget *widthLabel = new LabelWidget{this, textBoxRect(8, 0), "Width: "};
  QWidget *heightLabel = new LabelWidget{this, textBoxRect(8, 0), "Height: "};
  QWidget *formatLabel = new LabelWidget{this, textBoxRect(8, 0), "Format: "};
  layout->addWidget(widthLabel, 0, 0, Qt::AlignLeft);
  layout->addWidget(widthWidget, 0, 1, Qt::AlignRight);
  layout->addWidget(heightLabel, 1, 0, Qt::AlignLeft);
  layout->addWidget(heightWidget, 1, 1, Qt::AlignRight);
  layout->addWidget(formatLabel, 2, 0, Qt::AlignLeft);
  
  auto *formatLayout = new QHBoxLayout{};
  layout->addLayout(formatLayout, 3, 0, 1, 2, Qt::AlignCenter);
  formatLayout->setContentsMargins(0, 0, 0, 0);
  formatLayout->setSpacing(0);
  formatLayout->addStretch();
  for (FormatWidget *format : formatWidgets) {
    formatLayout->addWidget(format);
    formatLayout->addStretch();
  }
  
  layout->addWidget(okButton, 4, 0);
  layout->addWidget(cancelButton, 4, 1);
}

void InitCanvasDialog::connectSignals() {
  CONNECT(widthWidget, valueChanged, this, widthChanged);
  CONNECT(heightWidget, valueChanged, this, heightChanged);
  for (FormatWidget *format : formatWidgets) {
    CONNECT(format, checked, this, formatChanged);
  }
  CONNECT(okButton, pressed, this, accept);
  CONNECT(cancelButton, pressed, this, reject);
  CONNECT(this, accepted, this, finalize);
}

#include "init canvas dialog.moc"
