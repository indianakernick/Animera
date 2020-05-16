//
//  export pattern.cpp
//  Animera
//
//  Created by Indiana Kernick on 26/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "export pattern.hpp"

namespace {

template <typename Container, typename Value>
bool contains(const Container &c, const Value &v) {
  return std::find(std::begin(c), std::end(c), v) != std::end(c);
}

bool validFileNameChar(const QChar c) {
  constexpr QChar reserved[] = {'/', '\\', '?', '%', '*', ':', '|', '\"', '<', '>'};
  if (c.isLetterOrNumber()) return true;
  return !contains(reserved, c);
}

QString paddedNumber(const int zeros, const int value) {
  QString number = QString::number(value);
  while (number.size() < zeros) {
    number.push_front('0');
  }
  return number;
}

int countZeros(QString::const_iterator &begin, QString::const_iterator end) {
  int zeros = 0;
  while (begin != end && *begin == '0') {
    ++begin;
    ++zeros;
  }
  return zeros;
}

}

QString evalExportPattern(const QString &pattern, const ExportState state) {
  QString output;
  output.reserve(pattern.size());
  for (auto c = pattern.cbegin(); c != pattern.cend(); ++c) {
    if (*c == '%') {
      ++c;
      assert(c != pattern.cend());
      const int zeros = countZeros(c, pattern.cend());
      assert(c != pattern.cend());
      if (*c == 'L') {
        output.push_back(paddedNumber(zeros, +state.pos.l));
      } else if (*c == 'F') {
        output.push_back(paddedNumber(zeros, +state.pos.f));
      } else Q_UNREACHABLE();
    } else {
      assert(validFileNameChar(*c));
      output.push_back(*c);
    }
  }
  return output;
}

namespace {

Error checkExportPattern(const QString &pattern, bool &truncated) {
  if (pattern.isEmpty()) {
    truncated = true;
    return "Name pattern must not be empty";
  }
  for (auto c = pattern.cbegin(); c != pattern.cend(); ++c) {
    if (*c == '%') {
      ++c;
      if (c == pattern.cend()) {
        truncated = true;
        return "Name pattern contains truncated format sequence";
      }
      countZeros(c, pattern.cend());
      if (c == pattern.cend()) {
        truncated = true;
        return "Name pattern contains truncated format sequence";
      }
      if (*c != 'L' && *c != 'F') {
        return "Name pattern contains invalid format sequence";
      }
    } else {
      if (!validFileNameChar(*c)) {
        return QString{"Invalid character in name pattern ("} + *c + ")";
      }
    }
  }
  return {};
}

}

Error checkExportPattern(const QString &pattern) {
  bool truncated;
  return checkExportPattern(pattern, truncated);
}

ExportPatternValidator::ExportPatternValidator(QObject *parent)
  : QValidator{parent} {}

QString ExportPatternValidator::defaultText() const {
  return "sprite_%000F";
}

QValidator::State ExportPatternValidator::validate(QString &input, int &) const {
  bool truncated = false;
  if (checkExportPattern(input, truncated)) {
    return truncated ? State::Intermediate : State::Invalid;
  }
  return State::Acceptable;
}

void ExportPatternValidator::fixup(QString &input) const {
  if (input.isEmpty()) {
    input = defaultText();
    return;
  }
  for (auto c = input.cbegin(); c != input.cend();) {
    if (*c == '%') {
      const auto percentIter = c;
      ++c;
      if (c == input.cend()) {
        // input.erase(percentIter, c);
        input.chop(static_cast<int>(c - percentIter));
        return;
      }
      countZeros(c, input.cend());
      if (c == input.cend()) {
        // input.erase(percentIter, c);
        input.chop(static_cast<int>(c - percentIter));
        return;
      }
      if (*c != 'L' && *c != 'F') {
        // c = input.erase(percentIter, c + 1);
        const int idx = static_cast<int>(percentIter - input.cbegin());
        const int len = static_cast<int>(c - percentIter) + 1;
        input.remove(idx, len);
        c = input.cbegin() + idx;
      } else {
        ++c;
      }
    } else {
      if (!validFileNameChar(*c)) {
        // c = input.erase(c);
        const int idx = static_cast<int>(c - input.cbegin());
        input.remove(idx, 1);
        c = input.cbegin() + idx;
      } else {
        ++c;
      }
    }
  }
}
