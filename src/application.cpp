//
//  application.cpp
//  Animera
//
//  Created by Indiana Kernick on 10/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "application.hpp"

#include "window.hpp"
#include "connect.hpp"
#include "settings.hpp"
#include <QtGui/qevent.h>
#include "global font.hpp"
#include "config colors.hpp"
#include <QtWidgets/qtooltip.h>
#include "init canvas dialog.hpp"
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qdesktopwidget.h>

Application::Application(int &argc, char **argv)
  : QApplication{argc, argv} {
  setApplicationName("Animera");
  setOrganizationName("Indiana Kernick");
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
  dialog->open();
}

void Application::openFileDialog() {
  auto *dialog = new QFileDialog{desktop()};
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setAcceptMode(QFileDialog::AcceptOpen);
  dialog->setNameFilter("*.animera *.png");
  dialog->setFileMode(QFileDialog::ExistingFile);
  CONNECT(dialog, fileSelected, this, openFile);
  updateDirSettings(dialog, pref_animation_dir);
  // TODO: why can't I use open instead of exec?
  // https://forum.qt.io/topic/109616/how-to-open-an-asynchronous-application-modal-file-dialog
  dialog->exec();
}

void Application::windowClosed(Window *window) {
  windows.erase(std::remove(windows.begin(), windows.end(), window), windows.end());
}

bool Application::isClosing() const {
  return closing;
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
      "background-color:" + glob_main.name() + ";"
      "color: " + glob_light_2.name() + ";"
      "border-width: " + QString::number(glob_border_width) + "px;"
      "border-color: " + glob_border_color.name() + ";"
      "border-style: solid;"
    "}"
  );
  QToolTip::setFont(getGlobalFont());
}

Window *Application::makeWindow() {
  const Window *previous = windows.empty() ? nullptr : windows.back();
  return windows.emplace_back(new Window{desktop(), previous});
}

bool Application::event(QEvent *event) {
  switch (event->type()) {
    case QEvent::FileOpen:
      noFileTimer.stop();
      openFile(static_cast<QFileOpenEvent *>(event)->file());
      return true;
    case QEvent::Close:
      closing = true;
    default:
      return QApplication::event(event);
  }
}

#include "application.moc"
