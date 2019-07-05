//
//  application.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef application_hpp
#define application_hpp

#include "image.hpp"
#include <QtWidgets/qapplication.h>

class Window;

class Application : public QApplication {
  Q_OBJECT

public:
  Application(int &, char **);

  void newFileDialog();
  void openFileDialog();

private:
  void loadResources();
  Window *makeWindow();

private Q_SLOTS:
  void newFile(Format, QSize);
  void openFile(const QString &);
  
  bool event(QEvent *) override;
};

#endif
