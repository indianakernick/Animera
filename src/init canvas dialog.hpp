//
//  init canvas dialog.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 3/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef init_canvas_dialog_hpp
#define init_canvas_dialog_hpp

#include "image.hpp"
#include <QtWidgets/qdialog.h>
#include "color input widget.hpp"

class FormatWidget;
class TextPushButtonWidget;

class InitCanvasDialog final : public QDialog {
  Q_OBJECT
  
public:
  explicit InitCanvasDialog(QWidget *);
  
Q_SIGNALS:
  void canvasInitialized(Format, QSize);

private Q_SLOTS:
  void widthChanged(int);
  void heightChanged(int);
  void formatChanged(Format);
  void finalize();

private:
  NumberInputWidget widthWidget;
  NumberInputWidget heightWidget;
  std::vector<FormatWidget *> formatWidgets;
  TextPushButtonWidget *okButton = nullptr;
  TextPushButtonWidget *cancelButton = nullptr;
  QSize size = {128, 128};
  Format colorFormat = Format::rgba;
  
  void createWidgets();
  void setupLayout();
  void connectSignals();
};

#endif
