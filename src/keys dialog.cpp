//
//  keys dialog.cpp
//  Animera
//
//  Created by Indiana Kernick on 10/6/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "keys dialog.hpp"

#include "config keys.hpp"
#include "label widget.hpp"
#include "config colors.hpp"
#include "separator widget.hpp"
#include <QtWidgets/qboxlayout.h>

KeysDialog::KeysDialog(QWidget *parent)
  : Dialog{parent} {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("Key Bindings");
  setStyleSheet("background-color:" + glob_main.name());
  
  auto *rows = new QVBoxLayout{this};
  rows->setSpacing(0);
  rows->setContentsMargins(0, glob_margin, 0, glob_margin);
  
  addRow(rows, "Primary", key_primary);
  addRow(rows, "Secondary", key_secondary);
  addRow(rows, "Erase", key_tertiary);
  addRow(rows, "Sample", key_sample);
  
  addLine(rows);
  
  addRow(rows, "Tool Up", key_tool_up);
  addRow(rows, "Tool Down", key_tool_down);
  addRow(rows, "Zoom In", key_zoom_in);
  addRow(rows, "Zoom Out", key_zoom_out);
  addRow(rows, "Zoom Fit", key_zoom_fit);
  addRow(rows, "Undo", key_undo);
  addRow(rows, "Redo", key_redo);
  
  setFixedWidth(textBoxWidth(keys_width));
}

void KeysDialog::addLabel(QHBoxLayout *row, const QString &text) {
  const WidgetRect rect = basicRect(
    {glob_text_margin, glob_text_margin}, textBoxSize(text.size()), 0, 0
  );
  row->addWidget(new LabelWidget{this, rect, text});
}

void KeysDialog::addRow(QVBoxLayout *rows, const QString &name, const QKeySequence &key) {
  auto *row = new QHBoxLayout;
  rows->addLayout(row);
  row->setSpacing(0);
  row->setContentsMargins(glob_margin, 0, glob_margin, 0);
  addLabel(row, name);
  row->addStretch();
  addLabel(row, key.toString());
}

void KeysDialog::addLine(QVBoxLayout *rows) {
  rows->addWidget(new HoriSeparator{this});
}
