//
//  error dialog.hpp
//  Animera
//
//  Created by Indiana Kernick on 17/8/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef error_dialog_hpp
#define error_dialog_hpp

#include "dialog.hpp"

class ErrorDialog final : public Dialog {
public:
  ErrorDialog(QWidget *, const QString &, QString);
};

#endif
