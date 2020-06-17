//
//  file input widget.hpp
//  Animera
//
//  Created by Indiana Kernick on 24/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_file_input_widget_hpp
#define animera_file_input_widget_hpp

#include <QtWidgets/qwidget.h>

class PathInputWidget;
class FileInputButton;

class FileInputWidget final : public QWidget {
  Q_OBJECT
  
public:
  FileInputWidget(QWidget *, int);

  QString path() const;

public Q_SLOTS:
  void setPath(const QString &);

Q_SIGNALS:
  void pathChanged(const QString &);

private Q_SLOTS:
  void setPathFromDialog();
  void simplifyPath();
  void changePath();

private:
  PathInputWidget *text = nullptr;
  FileInputButton *icon = nullptr;

  void connectSignals();
};

#endif
