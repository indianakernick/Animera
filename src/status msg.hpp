//
//  status msg.hpp
//  Pixel 2
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

  void append(QPoint);
  void append(QSize);
  void append(SelectMode);
  void append(bool);
  void append(char);
  void append(int);
  
  template <size_t Size>
  void append(const char (&str)[Size]) {
    msg.append(str, Size - 1);
  }
  
  void appendLabeled(QPoint);
  void appendLabeled(QRect);
  void appendLabeled(SelectMode);

private:
  std::string msg;
};

#endif
