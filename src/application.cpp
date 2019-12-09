//
//  application.cpp
//  Animera
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "application.hpp"

#include "window.hpp"
#include "connect.hpp"
#include <QtGui/qevent.h>
#include "global font.hpp"
#include <QtCore/qresource.h>
#include <QtWidgets/qtooltip.h>
#include "init canvas dialog.hpp"
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qdesktopwidget.h>

Application::Application(int &argc, char **argv)
  : QApplication{argc, argv} {
  setApplicationName("Animera");
  setOrganizationName("Indiana Kernick");
  loadResources();
  initStyles();
}

void Application::waitForOpenEvent() {
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
  auto *dialog = new QFileDialog{desktop()};
  dialog->setAcceptMode(QFileDialog::AcceptOpen);
  dialog->setNameFilter("*.animera *.png");
  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setDirectory(QDir::homePath());
  CONNECT(dialog, fileSelected, this, openFile);
  dialog->show();
}

void Application::windowClosed(Window *window) {
  windows.erase(std::remove(windows.begin(), windows.end(), window), windows.end());
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
  if (path.endsWith(QLatin1String{".png"}, Qt::CaseInsensitive)) {
    return makeWindow()->openImage(path);
  }
  for (Window *window : windows) {
    if (window->windowFilePath() == path) {
      return raiseWindow(window);
    }
  }
  makeWindow()->openFile(path);
}

void Application::initStyles() {
  setStyleSheet(
    "QToolTip {"
      "background-color: " + glob_main.name() + ";"
      "color: " + glob_light_2.name() + ";"
      "border-width: " + QString::number(glob_border_width) + "px;"
      "border-color: " + glob_border_color.name() + ";"
      "border-style: solid;"
    "}"
  );
  QToolTip::setFont(getGlobalFont());
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
  const Window *previous = windows.empty() ? nullptr : windows.back();
  return windows.emplace_back(new Window{desktop(), previous});
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
