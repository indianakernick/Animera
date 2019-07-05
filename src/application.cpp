//
//  application.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 10/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "application.hpp"

#include <fstream>
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
  CONNECT(&noFileTimer, timeout, this, newFileDialog);
  noFileTimer.setSingleShot(true);
  noFileTimer.start(glob_launch_file_timeout_ms);
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
  return new Window{desktop(), desktop()->availableGeometry()};
}

void Application::newFile(const Format format, const QSize size) {
  Window *window = makeWindow();
  Q_EMIT window->newFile(format, size);
}

void Application::openFile(const QString &path) {
  Window *window = makeWindow();
  Q_EMIT window->openFile(path);
}

bool Application::event(QEvent *event) {
  if (event->type() == QEvent::FileOpen) {
    const int remaining = noFileTimer.remainingTime();
    noFileTimer.stop();
    std::ofstream log{"/Users/indikernick/Desktop/Test/log.txt"};
    log << "No file timeout: " << remaining << "ms" << std::endl;
    openFile(static_cast<QFileOpenEvent *>(event)->file());
    return true;
  } else {
    return QApplication::event(event);
  }
}

#include "application.moc"
