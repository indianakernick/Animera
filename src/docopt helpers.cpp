//
//  docopt helpers.cpp
//  Animera
//
//  Created by Indi Kernick on 30/11/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#include "docopt helpers.hpp"

#include "docopt.h"
#include "config.hpp"

QString nonZeroRangeStr(const IntRange range) {
  QString str = "\nValid range is: [";
  str += QString::number(range.min);
  str += ", -1] U [1, ";
  str += QString::number(range.max);
  str += ']';
  return str;
}

QString rangeStr(const IntRange range) {
  QString str = "\nValid range is: [";
  str += QString::number(range.min);
  str += ", ";
  str += QString::number(range.max);
  str += ']';
  return str;
}

Error setInt(long &number, const docopt::value &value, const QString &name) {
  try {
    number = value.asLong();
  } catch (std::exception &) {
    return name + " must be an integer";
  }
  return {};
}

Error setInt(
  int &number,
  const docopt::value &value,
  const QString &name,
  const IntRange range
) {
  long longNumber;
  TRY(setInt(longNumber, value, name));
  if (longNumber < range.min || longNumber > range.max) {
    return name + " is out of range" + rangeStr(range);
  }
  number = static_cast<int>(longNumber);
  return {};
}

Error setNonZeroInt(
  int &number,
  const docopt::value &value,
  const QString &name,
  const IntRange range
) {
  long longNumber;
  TRY(setInt(longNumber, value, name));
  if (longNumber == 0) {
    return name + " cannot be 0" + nonZeroRangeStr(range);
  }
  if (longNumber < expt_stride.min || longNumber > expt_stride.max) {
    return name + " is out of range" + nonZeroRangeStr(range);
  }
  number = static_cast<int>(longNumber);
  return {};
}
