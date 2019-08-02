//
//  sprite.hpp
//  Animera
//
//  Created by Indi Kernick on 7/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef sprite_hpp
#define sprite_hpp

#include "palette.hpp"
#include "timeline.hpp"
#include <QtCore/qobject.h>

struct ExportOptions;

class Sprite final : public QObject {
  Q_OBJECT

public Q_SLOTS:
  void newFile(Format, QSize);
  void saveFile(const QString &) const;
  void openFile(const QString &);
  void exportSprite(const ExportOptions &) const;
  
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
