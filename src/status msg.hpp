//
//  status msg.hpp
//  Animera
//
//  Created by Indiana Kernick on 27/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_status_msg_hpp
#define animera_status_msg_hpp

#include <string>
#include <string_view>
#include <QtCore/qrect.h>
#include "paint params.hpp"

class StatusMsg {
public:
  void clear();
  bool empty() const;
  std::size_t size() const;
  std::string_view get() const;

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
  
  template <std::size_t Size>
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
