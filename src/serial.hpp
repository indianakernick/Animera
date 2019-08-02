//
//  serial.hpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef serial_hpp
#define serial_hpp

#include <QtCore/qiodevice.h>

template <typename T>
void serializeBytes(QIODevice *dev, const T &data) {
  assert(dev);
  assert(dev->isWritable());
  dev->write(reinterpret_cast<const char *>(&data), sizeof(T));
}

template <typename T>
void deserializeBytes(QIODevice *dev, T &data) {
  assert(dev);
  assert(dev->isReadable());
  dev->read(reinterpret_cast<char *>(&data), sizeof(T));
}

template <typename T>
T deserializeBytesAs(QIODevice *dev) {
  T data;
  deserializeBytes(dev, data);
  return data;
}

#endif
