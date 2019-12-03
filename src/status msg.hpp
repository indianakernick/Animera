//
//  status msg.hpp
//  Animera
//
//  Created by Indi Kernick on 27/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef status_msg_hpp
#define status_msg_hpp

#include <string>
#include <string_view>
#include <QtCore/qrect.h>
#include "paint params.hpp"

class ToolCtx;

class StatusMsg {
public:
  [[deprecated]] StatusMsg();
  explicit StatusMsg(const ToolCtx *);
  ~StatusMsg();

  void clear();
  bool empty() const;
  [[deprecated]] std::string_view get() const;

  StatusMsg &append(bool);
  StatusMsg &append(char);
  StatusMsg &append(int);
  StatusMsg &append(int, int);
  StatusMsg &append(int, int, int, int);
  StatusMsg &append(QPoint);
  StatusMsg &append(QSize);
  StatusMsg &append(QRect);
  StatusMsg &append(SelectMode);
  StatusMsg &append(LineGradMode);
  
  template <size_t Size>
  StatusMsg &append(const char (&str)[Size]) {
    msg.append(str, Size - 1);
    return *this;
  }
  
  StatusMsg &appendLabeled(QPoint);
  StatusMsg &appendLabeled(SelectMode);
  StatusMsg &appendLabeled(LineGradMode);

private:
  const ToolCtx *ctx;
  std::string msg;
};

#endif
