//
//  file input widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 24/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef file_input_widget_hpp
#define file_input_widget_hpp

#include <QtWidgets/qwidget.h>

class TextInputWidget;
class FileInputButton;
class DirValidator;

class FileInputWidget final : public QWidget {
  Q_OBJECT
  
public:
  FileInputWidget(QWidget *, int);

  QString path() const;

Q_SIGNALS:
  void pathChanged(const QString &);

private Q_SLOTS:
  void setPathFromDialog();
  void setPath(const QString &);
  void simplifyPath();
  void changePath();

private:
  TextInputWidget *text = nullptr;
  FileInputButton *icon = nullptr;
  DirValidator *validator = nullptr;

  void initText();
  void connectSignals();
};

#endif
