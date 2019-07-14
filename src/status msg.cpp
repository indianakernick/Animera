//
//  status msg.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 27/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "status msg.hpp"

#include "geometry.hpp"

void StatusMsg::clear() {
  msg.clear();
}

bool StatusMsg::empty() const {
  return msg.empty();
}

std::string_view StatusMsg::get() const {
  return msg;
}

StatusMsg &StatusMsg::append(const QPoint point) {
  append('[');
  append(point.x());
  append(' ');
  append(point.y());
  append(']');
  return *this;
}

StatusMsg &StatusMsg::append(const QSize size) {
  return append(toPoint(size));
}

StatusMsg &StatusMsg::append(const SelectMode mode) {
  switch (mode) {
    case SelectMode::copy:  return append("COPY");
    case SelectMode::paste: return append("PASTE");
    default: Q_UNREACHABLE();
  }
}

StatusMsg &StatusMsg::append(const bool b) {
  msg += b ? "YES" : "NO";
  return *this;
}

StatusMsg &StatusMsg::append(const char c) {
  msg += c;
  return *this;
}

StatusMsg &StatusMsg::append(const int i) {
  msg += std::to_string(i);
  return *this;
}

StatusMsg &StatusMsg::appendLabeled(const QPoint pos) {
  append("POS: ");
  append(pos);
  return *this;
}

StatusMsg &StatusMsg::appendLabeled(const QRect rect) {
  append("POS: ");
  append(rect.topLeft());
  append(" SIZE: ");
  append(rect.size());
  return *this;
}

StatusMsg &StatusMsg::appendLabeled(const SelectMode mode) {
  append(mode);
  append(" - ");
  return *this;
}
