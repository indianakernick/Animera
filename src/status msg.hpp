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

class StatusMsg {
public:
  void clear();
  bool empty() const;
  std::string_view get() const;

  StatusMsg &append(QPoint);
  StatusMsg &append(QSize);
  StatusMsg &append(QRect);
  StatusMsg &append(SelectMode);
  StatusMsg &append(LineGradMode);
  StatusMsg &append(bool);
  StatusMsg &append(char);
  StatusMsg &append(int);
  
  template <size_t Size>
  StatusMsg &append(const char (&str)[Size]) {
    msg.append(str, Size - 1);
    return *this;
  }
  
  StatusMsg &appendLabeled(QPoint);
  StatusMsg &appendLabeled(SelectMode);
  StatusMsg &appendLabeled(LineGradMode);

private:
  std::string msg;
};

#endif
