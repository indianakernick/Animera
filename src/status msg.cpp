//
//  status msg.cpp
//  Animera
//
//  Created by Indiana Kernick on 27/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "status msg.hpp"

void StatusMsg::clear() {
  msg.clear();
}

bool StatusMsg::empty() const {
  return msg.empty();
}

std::size_t StatusMsg::size() const {
  return msg.size();
}

std::string_view StatusMsg::get() const {
  return msg;
}

StatusMsg &StatusMsg::append(const bool b) {
  msg += b ? "Yes" : "No";
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

StatusMsg &StatusMsg::append(const int a, const int b) {
  append('[');
  append(a);
  append(' ');
  append(b);
  append(']');
  return *this;
}

StatusMsg &StatusMsg::append(const int a, const int b, const int c, const int d) {
  append('[');
  append(a);
  append(' ');
  append(b);
  append(' ');
  append(c);
  append(' ');
  append(d);
  append(']');
  return *this;
}

StatusMsg &StatusMsg::append(const QPoint point) {
  return append(point.x(), point.y());
}

StatusMsg &StatusMsg::append(const QSize size) {
  return append(size.width(), size.height());
}

StatusMsg &StatusMsg::append(const QRect rect) {
  append(rect.topLeft());
  append(' ');
  append(rect.size());
  return *this;
}

StatusMsg &StatusMsg::append(const SelectMode mode) {
  switch (mode) {
    case SelectMode::copy:  return append("Copy");
    case SelectMode::paste: return append("Paste");
    default: Q_UNREACHABLE();
  }
}

StatusMsg &StatusMsg::append(const LineGradMode mode) {
  switch (mode) {
    case LineGradMode::hori: return append("Hori");
    case LineGradMode::vert: return append("Vert");
    default: Q_UNREACHABLE();
  }
}

StatusMsg &StatusMsg::appendLabeled(const QPoint pos) {
  append("Pos: ");
  append(pos);
  return *this;
}

StatusMsg &StatusMsg::appendLabeled(const SelectMode mode) {
  append(mode);
  append(" - ");
  return *this;
}

StatusMsg &StatusMsg::appendLabeled(const LineGradMode mode) {
  append(mode);
  append(" - ");
  return *this;
}
