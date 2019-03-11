//
//  application.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "application.hpp"

#include <QtCore/qresource.h>
#include <QtWidgets/qdesktopwidget.h>

Application::Application(int &argc, char **argv)
  : QApplication{argc, argv} {
  QString resPath = applicationDirPath();
  #if defined(Q_OS_MACOS)
  resPath += "/../Resources/";
  #elif defined(Q_OS_WIN)
  resPath += "/";
  #else
  resPath += "/";
  #endif
  [[maybe_unused]] bool registered = QResource::registerResource(resPath + "resources.rcc");
  assert(registered);
  window.emplace(desktop()->availableGeometry());
}

bool Application::event(QEvent *event) {
  if (event->type() == QEvent::FileOpen) {
    //window.fileOpen(static_cast<QFileOpenEvent *>(event));
    return true;
  } else {
    return QApplication::event(event);
  }
}

#include "application.moc"
