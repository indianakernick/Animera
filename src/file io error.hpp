//
//  file io error.hpp
//  Animera
//
//  Created by Indi Kernick on 27/10/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef file_io_error_hpp
#define file_io_error_hpp

#include <exception>

class FileIOError final : public std::exception {
public:
  const char *what() const noexcept override {
    return "File IO error";
  }
};

#endif