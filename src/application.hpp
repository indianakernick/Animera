//
//  application.hpp
//  Animera
//
//  Created by Indiana Kernick on 10/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef application_hpp
#define application_hpp

#include "image.hpp"
#include <QtCore/qtimer.h>
#include <QtWidgets/qapplication.h>

class Window;

class Application : public QApplication {
  Q_OBJECT

public:
  Application(int &, char **);

  void waitForOpenEvent();
  void newFileDialog();
  void openFileDialog();
  void windowClosed(Window *);
  bool isClosing() const;

public Q_SLOTS:
  void newFile(Format, QSize);
  void openFile(const QString &);

private:
  std::vector<Window *> windows;
  QTimer noFileTimer;
  bool closing = false;

  void initStyles();
  Window *makeWindow();
  
  bool event(QEvent *) override;
};

#endif
