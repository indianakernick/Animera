//
//  dialog.hpp
//  Animera
//
//  Created by Indiana Kernick on 3/8/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_dialog_hpp
#define animera_dialog_hpp

#include <QtWidgets/qdialog.h>

class Dialog : public QDialog {
public:
  explicit Dialog(QWidget *);

private:
  void keyPressEvent(QKeyEvent *) override;
};

#endif
