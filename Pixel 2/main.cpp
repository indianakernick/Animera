//
//  main.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include <iostream>
#include <QtGui/qevent.h>
#include <QtWidgets/qmenubar.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qapplication.h>

class Application : public QApplication {
public:
  Application(int &argc, char **argv)
    : QApplication{argc, argv},
      button{"Application was not launched with file"} {
    button.show();
    QMenu *file = menubar.addMenu("File");
    QAction *open = file->addAction("Open");
    QAction *save = file->addAction("Save");
    file->addSeparator();
    QAction *exp = file->addAction("Export");
    connect(open, &QAction::triggered, this, &Application::openDoc);
  }
  
private:
  QMainWindow window;
  QPushButton button;
  QMenuBar menubar;

  void openDoc() {
    std::cout << "Open document\n";
  }

  bool event(QEvent *event) override {
    switch (event->type()) {
      case QEvent::FileOpen:
        button.setText(static_cast<QFileOpenEvent *>(event)->file());
        return true;
      default:
        return QApplication::event(event);
    }
  }
};

int main(int argc, char **argv) {
  Application app{argc, argv};
  return app.exec();
}
