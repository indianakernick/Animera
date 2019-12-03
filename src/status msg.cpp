//
//  status msg.cpp
//  Animera
//
//  Created by Indi Kernick on 27/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "status msg.hpp"

#include "tool.hpp"

StatusMsg::StatusMsg()
  : ctx{} {}

StatusMsg::StatusMsg(const ToolCtx *ctx)
  : ctx{ctx} {
  assert(ctx);
}

StatusMsg::~StatusMsg() {
  if (ctx) {
    Q_EMIT ctx->shouldShowNorm(msg);
  }
}

void StatusMsg::clear() {
  msg.clear();
}

bool StatusMsg::empty() const {
  return msg.empty();
}

std::string_view StatusMsg::get() const {
  return msg;
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
    case SelectMode::copy:  return append("COPY");
    case SelectMode::paste: return append("PASTE");
    default: Q_UNREACHABLE();
  }
}

StatusMsg &StatusMsg::append(const LineGradMode mode) {
  switch (mode) {
    case LineGradMode::hori: return append("HORI");
    case LineGradMode::vert: return append("VERT");
    default: Q_UNREACHABLE();
  }
}

StatusMsg &StatusMsg::appendLabeled(const QPoint pos) {
  append("POS: ");
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
