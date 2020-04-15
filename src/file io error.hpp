//
//  file io error.hpp
//  Animera
//
//  Created by Indiana Kernick on 27/10/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_file_io_error_hpp
#define animera_file_io_error_hpp

#include <exception>
#include <QtCore/qiodevice.h>

class FileIOError final : public std::runtime_error {
public:
  explicit FileIOError(const QIODevice &dev)
    : std::runtime_error{dev.errorString().toStdString()},
      error{dev.errorString()} {}

  QString msg() const {
    return error;
  }

private:
  QString error;
};

#endif
