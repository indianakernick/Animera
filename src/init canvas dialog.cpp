//
//  init canvas dialog.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "init canvas dialog.hpp"

#include "connect.hpp"
#include "global font.hpp"
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include "label widget.hpp"
#include "radio button widget.hpp"
#include <QtWidgets/qgridlayout.h>

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

class TextButton final : public QAbstractButton {
public:
  TextButton(QWidget *parent, const WidgetRect rect, const QString &text)
    : QAbstractButton{parent}, rect{rect}, text{text} {
    setFixedSize(rect.widget().size());
  }

private:
  WidgetRect rect;
  QString text;

  void paintEvent(QPaintEvent *) override {
    QPainter painter{this};
    painter.fillRect(rect.outer(), glob_light_1);
    painter.setFont(getGlobalFont());
    painter.setBrush(Qt::NoBrush);
    painter.setPen(glob_text_color);
    painter.drawText(rect.inner(), Qt::AlignCenter, text);
  }
};

InitCanvasDialog::InitCanvasDialog(QWidget *widget)
  : QDialog{widget},
    widthWidget{this, textBoxRect(5, 0), 128, 65536},
    heightWidget{this, textBoxRect(5, 0), 128, 65536} {
  setWindowTitle("New File");
  setStyleSheet("background-color:" + glob_main.name());
  formatWidgets.push_back(new FormatWidget{this, ":/Formats/rgba.png", Format::color});
  formatWidgets.push_back(new FormatWidget{this, ":/Formats/gray.png", Format::gray});
  formatWidgets.push_back(new FormatWidget{this, ":/Formats/index.png", Format::palette});
  formatWidgets[0]->setToolTip("32-bit RGBA");
  formatWidgets[1]->setToolTip("8-bit Grayscale");
  formatWidgets[2]->setToolTip("8-bit Indexed");
  formatWidgets.front()->setChecked(true);
  okButton = new TextButton{this, textBoxRect(8, 0), "Ok"};
  cancelButton = new TextButton{this, textBoxRect(8, 0), "Cancel"};
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

void InitCanvasDialog::setupLayout() {
  QGridLayout *layout = new QGridLayout{this};
  setLayout(layout);
  layout->setSpacing(0);
  layout->setContentsMargins(1_px, 1_px, 1_px, 1_px);
  QWidget *widthLabel = new LabelWidget{this, textBoxRect(8, 0_px), "Width: "};
  QWidget *heightLabel = new LabelWidget{this, textBoxRect(8, 0_px), "Height: "};
  QWidget *formatLabel = new LabelWidget{this, textBoxRect(8, 0_px), "Format: "};
  layout->addWidget(widthLabel, 0, 0, Qt::AlignLeft);
  layout->addWidget(&widthWidget, 0, 1, Qt::AlignRight);
  layout->addWidget(heightLabel, 1, 0, Qt::AlignLeft);
  layout->addWidget(&heightWidget, 1, 1, Qt::AlignRight);
  layout->addWidget(formatLabel, 2, 0, Qt::AlignLeft);
  
  QHBoxLayout *formatLayout = new QHBoxLayout{};
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
  CONNECT(&widthWidget, valueChanged, this, widthChanged);
  CONNECT(&heightWidget, valueChanged, this, heightChanged);
  for (FormatWidget *format : formatWidgets) {
    CONNECT(format, checked, this, formatChanged);
  }
  CONNECT(okButton, pressed, this, accept);
  CONNECT(cancelButton, pressed, this, reject);
  CONNECT(this, accepted, this, finalize);
}

#include "init canvas dialog.moc"
