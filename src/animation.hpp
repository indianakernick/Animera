//
//  animation.hpp
//  Animera
//
//  Created by Indiana Kernick on 7/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_animation_hpp
#define animera_animation_hpp

#include "error.hpp"
#include "palette.hpp"
#include "timeline.hpp"
#include <QtCore/qobject.h>

struct ExportOptions;

class Animation final : public QObject {
  Q_OBJECT

public Q_SLOTS:
  void optimize();
  void newFile(Format, QSize);
  Error saveFile(const QString &) const;
  Error openFile(const QString &);
  Error openImage(const QString &);
  
Q_SIGNALS:
  void canvasInitialized(Format, QSize);

public:
  Timeline timeline;
  Palette palette;
  
  Format getFormat() const;
  QSize getSize() const;

private:
  Format format;
  QSize size;
};

#endif
