//
//  keys dialog.hpp
//  Animera
//
//  Created by Indiana Kernick on 10/6/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_keys_dialog_hpp
#define animera_keys_dialog_hpp

#include "dialog.hpp"

class QVBoxLayout;
class QHBoxLayout;

class KeysDialog final : public Dialog {
public:
  explicit KeysDialog(QWidget *);
  
private:
  void addLabel(QHBoxLayout *, const QString &);
  void addRow(QVBoxLayout *, const QString &, const QKeySequence &);
  void addLine(QVBoxLayout *);
};

#endif
