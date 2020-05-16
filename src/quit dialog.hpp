//
//  quit dialog.hpp
//  Animera
//
//  Created by Indiana Kernick on 8/12/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_quit_dialog_hpp
#define animera_quit_dialog_hpp

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
