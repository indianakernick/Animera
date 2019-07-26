//
//  export pattern.cpp
//  Pixel 2
//
//  Created by Indi Kernick on 26/7/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "export pattern.hpp"

namespace {

bool validFileNameChar(const QChar c) {
  constexpr QChar reserved[] = {'/', '\\', '?', '%', '*', ':', '|', '\"', '<', '>'};
  if (c.isLetterOrNumber()) return true;
  return (std::find(std::begin(reserved), std::end(reserved), c) == std::end(reserved));
}

QString paddedNumber(const int zeros, const int value) {
  QString number = QString::number(value);
  while (number.size() < zeros) {
    number.push_front('0');
  }
  return number;
}

}

QString evalExportPattern(
  const QString &pattern,
  const LayerIdx layer,
  const FrameIdx frame
) {
  QString output;
  output.reserve(pattern.size());
  const auto last = pattern.end() - 1;
  for (auto c = pattern.begin(); c != pattern.end(); ++c) {
    if (*c == '%') {
      assert(c != last);
      ++c;
      int zeros = 0;
      while (c != last && *c == '0') {
        ++c;
        ++zeros;
      }
      if (*c == 'L') {
        output.push_back(paddedNumber(zeros, layer));
      } else if (*c == 'F') {
        output.push_back(paddedNumber(zeros, frame));
      } else Q_UNREACHABLE();
    } else {
      assert(validFileNameChar(*c));
      output.push_back(*c);
    }
  }
  return output;
}
