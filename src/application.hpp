//
//  application.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef application_hpp
#define application_hpp

#include <optional>
#include "window.hpp"
#include <QtWidgets/qapplication.h>

class Application : public QApplication {
  Q_OBJECT

public:
  Application(int &, char **);
  
private:
  std::optional<Window> window;
  
  bool event(QEvent *) override;
};

#endif
