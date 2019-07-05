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
#include "image.hpp"
#include "window.hpp"
#include "init canvas dialog.hpp"
#include <QtWidgets/qapplication.h>

class Application : public QApplication {
  Q_OBJECT

public:
  Application(int &, char **);

private:
  std::optional<Window> window;
  std::optional<InitCanvasDialog> initDialog;
  
  void loadResources();

private Q_SLOTS:
  void initCanvas(Format, QSize);
  
  bool event(QEvent *) override;
};

#endif
