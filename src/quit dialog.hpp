//
//  quit dialog.hpp
//  Animera
//
//  Created by Indi Kernick on 8/12/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef quit_dialog_hpp
#define quit_dialog_hpp

#include "dialog.hpp"

class TextPushButtonWidget;

class QuitDialog final : public Dialog {
  Q_OBJECT

public:
  explicit QuitDialog(QWidget *);

Q_SIGNALS:
  void shouldQuit();
  void shouldSave();
  void shouldCancel();

private:
  TextPushButtonWidget *quit = nullptr;
  TextPushButtonWidget *save = nullptr;
  TextPushButtonWidget *cancel = nullptr;
  
  void createWidgets();
  void setupLayout();
  void connectSignals();
};

#endif
