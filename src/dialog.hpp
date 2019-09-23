//
//  dialog.hpp
//  Animera
//
//  Created by Indi Kernick on 3/8/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef dialog_hpp
#define dialog_hpp

#include <QtWidgets/qdialog.h>

class Dialog : public QDialog {
public:
  explicit Dialog(QWidget *);

  void open() override;

private:
  void keyPressEvent(QKeyEvent *) override;
};

#endif
