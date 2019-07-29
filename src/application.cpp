//
//  application.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "application.hpp"

#include "window.hpp"
#include "connect.hpp"
#include <QtGui/qevent.h>
#include <QtCore/qresource.h>
#include "init canvas dialog.hpp"
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qdesktopwidget.h>

Application::Application(int &argc, char **argv)
  : QApplication{argc, argv} {
  loadResources();
  CONNECT(noFileTimer, timeout, this, newFileDialog);
  noFileTimer.setSingleShot(true);
  noFileTimer.start(0);
}

void Application::newFileDialog() {
  auto *dialog = new InitCanvasDialog{desktop()};
  CONNECT(dialog, canvasInitialized, this, newFile);
  dialog->show();
}

void Application::openFileDialog() {
  auto *dialog = new QFileDialog{desktop(), "Open File"};
  CONNECT(dialog, fileSelected, this, openFile);
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setNameFilter("Pixel 2 File (*.px2)");
  dialog->show();
}

void Application::windowClosed(Window *window) {
  windows.erase(std::remove(windows.begin(), windows.end(), window), windows.end());
}

void Application::loadResources() {
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
}

Window *Application::makeWindow() {
  return windows.emplace_back(
    new Window{desktop(), desktop()->availableGeometry()}
  );
}

void Application::newFile(const Format format, const QSize size) {
  makeWindow()->newFile(format, size);
}

namespace {

void raiseWindow(QMainWindow *window) {
  const Qt::WindowStates state = window->windowState();
  window->setWindowState((state & ~Qt::WindowMinimized) | Qt::WindowActive);
  window->activateWindow();
  window->raise();
}

}

void Application::openFile(const QString &path) {
  for (Window *window : windows) {
    if (window->windowFilePath() == path) {
      return raiseWindow(window);
    }
  }
  makeWindow()->openFile(path);
}

bool Application::event(QEvent *event) {
  if (event->type() == QEvent::FileOpen) {
    noFileTimer.stop();
    openFile(static_cast<QFileOpenEvent *>(event)->file());
    return true;
  } else {
    return QApplication::event(event);
  }
}

#include "application.moc"
