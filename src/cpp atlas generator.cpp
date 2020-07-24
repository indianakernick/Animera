//
//  cpp atlas generator.cpp
//  Animera
//
//  Created by Indiana Kernick on 18/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "cpp atlas generator.hpp"

#include "strings.hpp"
#include "file io.hpp"
#include "composite.hpp"
#include <QtCore/qdir.h>
#include "export png.hpp"
#include <QtCore/qmath.h>
#include <QtCore/qtextstream.h>

namespace {

constexpr char sprite_id_operators[] = R"(
[[nodiscard]] inline SpriteRect getSpriteRect(const SpriteID id) noexcept {
  assert(0 <= static_cast<int>(id));
  assert(static_cast<int>(id) < static_cast<int>(SpriteID::count_));
  return sprite_rects[static_cast<int>(id)];
}

[[nodiscard]] constexpr SpriteID operator+(SpriteID id, const int off) noexcept {
  assert(0 <= static_cast<int>(id));
  assert(static_cast<int>(id) < static_cast<int>(SpriteID::count_));
  id = SpriteID{static_cast<int>(id) + off};
  assert(0 <= static_cast<int>(id));
  assert(static_cast<int>(id) < static_cast<int>(SpriteID::count_));
  return id;
}

[[nodiscard]] constexpr SpriteID operator-(const SpriteID id, const int off) noexcept {
  return id + -off;
}

[[nodiscard]] constexpr int operator-(const SpriteID a, const SpriteID b) noexcept {
  assert(0 <= static_cast<int>(a));
  assert(static_cast<int>(a) <= static_cast<int>(SpriteID::count_));
  assert(0 <= static_cast<int>(b));
  assert(static_cast<int>(b) < static_cast<int>(SpriteID::count_));
  return static_cast<int>(a) - static_cast<int>(b);
}

constexpr SpriteID &operator+=(SpriteID &id, const int off) noexcept {
  id = id + off;
  return id;
}

constexpr SpriteID &operator-=(SpriteID &id, const int off) noexcept {
  id = id - off;
  return id;
}

constexpr SpriteID &operator++(SpriteID &id) noexcept {
  return id += 1;
}

constexpr SpriteID &operator--(SpriteID &id) noexcept {
  return id -= 1;
}

constexpr SpriteID operator++(SpriteID &id, int) noexcept {
  const SpriteID copy = id;
  id += 1;
  return copy;
}

constexpr SpriteID operator--(SpriteID &id, int) noexcept {
  const SpriteID copy = id;
  id -= 1;
  return copy;
}
)";

constexpr char sprite_rect_def[] = R"(
struct alignas(std::uint64_t) SpriteRect {
  std::uint16_t x = 0, y = 0;
  std::uint16_t w = 0, h = 0;
};
)";

constexpr char sprite_rect_operators[] = R"(
[[nodiscard]] constexpr bool operator==(const SpriteRect a, const SpriteRect b) noexcept {
  return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

[[nodiscard]] constexpr bool operator!=(const SpriteRect a, const SpriteRect b) noexcept {
  return !(a == b);
}
)";

bool convertToIdentifier(QString &str) {
  for (QChar &ch : str) {
    if (!ch.isLetterOrNumber()) {
      ch = '_';
    }
  }
  if (str.isEmpty() || str.front().isDigit()) {
    str.prepend('_');
    return true;
  } else {
    return false;
  }
}

}

Error CppAtlasGenerator::initAtlas(PixelFormat format, const QString &name, const QString &dir) {
  if (format == PixelFormat::index) {
    return "C++ Atlas Generator does not support indexed pixel format";
  }
  if (format == PixelFormat::monochrome) {
    return "C++ Atlas Generator does not support monochrome pixel format";
  }
  
  packer.init(format);
  enumeration = "  null_ = 0,\n";
  array = "  SpriteRect{},\n";
  names.clear();
  names.insert("null_");
  collision.clear();
  atlasDir = dir;
  atlasName = name;
  return {};
}

void CppAtlasGenerator::addName(
  const std::size_t i,
  const SpriteNameParams &params,
  const SpriteNameState &state
) {
  QString name = params.baseName;
  int baseName = name.size();
  appendLayerName(name, params, state);
  int layerName = name.size();
  appendGroupName(name, params, state);
  int groupName = name.size();
  appendFrameName(name, params, state);
  int frameName = name.size();
  
  if (convertToIdentifier(name)) {
    ++baseName;
    ++layerName;
    ++groupName;
    ++frameName;
  }
  
  const bool hasLayerName = layerName > baseName && baseName > 0;
  const bool hasGroupName = groupName > layerName && layerName > 0;
  const bool hasFrameName = frameName > groupName && groupName > 0;
  
  if (state.frame == state.groupBegin) {
    if (state.group == GroupIdx{0}) {
      if (hasLayerName && state.layer == LayerIdx{0}) {
        addAlias(name.left(baseName), "beg_", i);
      }
      if (hasGroupName) {
        addAlias(name.left(layerName), "beg_", i);
      }
    }
    if (hasFrameName) {
      addAlias(name.left(groupName), "beg_", i);
    }
  }
  
  appendEnumerator(name, i);
  insertName(name);
  
  if (state.frame - state.groupBegin == state.frameCount - FrameIdx{1}) {
    if (hasFrameName) {
      addAlias(name.left(groupName), "end_", i + 1);
    }
    if (state.group == state.groupCount - GroupIdx{1}) {
      if (hasGroupName) {
        addAlias(name.left(layerName), "end_", i + 1);
      }
      if (hasLayerName && state.layer == state.layerCount - LayerIdx{1}) {
        addAlias(name.left(baseName), "end_", i + 1);
      }
    }
  }
}

void CppAtlasGenerator::addSize(const QSize size) {
  packer.append(size);
}

void CppAtlasGenerator::addWhiteName() {
  appendEnumerator("whitepixel_", packer.count());
  insertName("whitepixel_");
  packer.appendWhite();
}

QString CppAtlasGenerator::hasNameCollision() {
  return collision;
}

Error CppAtlasGenerator::packRectangles() {
  return packer.pack();
}

Error CppAtlasGenerator::initAnimation(const Format format, const PaletteCSpan palette) {
  return packer.setFormat(format, palette);
}

Error CppAtlasGenerator::addImage(const std::size_t i, const QImage &img) {
  if (img.isNull()) {
    appendRectangle({});
  } else {
    appendRectangle(packer.copy(i, img));
  }
  return {};
}

Error CppAtlasGenerator::addWhiteImage() {
  appendRectangle(packer.copyWhite(packer.count() - 1));
  return {};
}

Error CppAtlasGenerator::finalize() {
  appendEnumerator("count_", packer.count());
  TRY(writeCpp());
  return writeHpp();
}

void CppAtlasGenerator::addAlias(QString base, const char *alias, const std::size_t value) {
  if (!base.isEmpty()) base += '_';
  base += alias;
  appendEnumerator(base, value);
  insertName(base);
}

void CppAtlasGenerator::appendEnumerator(const QString &name, const std::size_t value) {
  enumeration += "  ";
  enumeration += name;
  enumeration += " = ";
  enumeration += QString::number(value + 1);
  enumeration += ",\n";
}

void CppAtlasGenerator::appendRectangle(const QRect &rect) {
  if (rect.isEmpty()) {
    array += "  SpriteRect{},\n";
  } else {
    array += "  SpriteRect{";
    array += QString::number(rect.x());
    array += ", ";
    array += QString::number(rect.y());
    array += ", ";
    array += QString::number(rect.width());
    array += ", ";
    array += QString::number(rect.height());
    array += "},\n";
  }
}

void CppAtlasGenerator::insertName(const QString &name) {
  if (collision.isEmpty() && !names.insert(name).second) {
    collision = name;
  }
}

Error CppAtlasGenerator::writeBytes(QIODevice &dev, const char *data, const std::size_t size) {
  constexpr int bytes_per_line = (80 - 2) / 6;
  constexpr char hex_chars[] = "0123456789ABCDEF";
  char hex[] = "0x00, ";

  for (std::size_t i = 0; i != size; ++i) {
    if (i % bytes_per_line == 0) {
      if (dev.write("\n  ", 3) != 3) {
        return dev.errorString();
      }
    }
    
    static_assert(CHAR_BIT == 8);
    const unsigned char byte = data[i];
    hex[2] = hex_chars[byte >> 4];
    hex[3] = hex_chars[byte & 15];
    if (dev.write(hex, 6) != 6) {
      return dev.errorString();
    }
  }
  
  return {};
}

Error CppAtlasGenerator::writeCpp() {
  QBuffer textureBuffer;
  textureBuffer.open(QIODevice::ReadWrite);
  TRY(packer.writePng(textureBuffer));
  
  QString nameSpace = atlasName;
  convertToIdentifier(nameSpace);
  
  FileWriter writer;
  TRY(writer.open(atlasDir + QDir::separator() + atlasName + ".cpp"));
  writer.dev().setTextModeEnabled(true);
  QTextStream stream{&writer.dev()};
  stream << "// This file was generated by Animera\n";
  stream << '\n';
  stream << "#include <cstddef>\n";
  stream << "#include <cstdint>\n";
  stream << '\n';
  stream << "namespace animera {\n";
  stream << sprite_rect_def;
  stream << '\n';
  stream << "inline namespace " << nameSpace << " {\n";
  stream << '\n';
  stream << "extern const int texture_width = " << packer.width() << ";\n";
  stream << "extern const int texture_height = " << packer.height() << ";\n";
  stream << "extern const std::size_t texture_size = " << textureBuffer.size() << ";\n";
  stream << '\n';
  stream << "extern const unsigned char texture_data[] = {";
  stream.flush();
  
  QByteArray &textureArray = textureBuffer.buffer();
  TRY(writeBytes(writer.dev(), textureArray.data(), static_cast<std::size_t>(textureArray.size())));
  stream << '\n';
  stream << "};\n";
  stream << '\n';
  stream << "extern const SpriteRect sprite_rects[] = {\n";
  stream << array;
  stream << "};\n";
  stream << '\n';
  stream << "}\n";
  stream << '\n';
  stream << "}\n";
  stream.flush();
  
  if (stream.status() == QTextStream::WriteFailed) {
    return "Error writing to file";
  }
  
  return writer.flush();
}

Error CppAtlasGenerator::writeHpp() {
  FileWriter writer;
  TRY(writer.open(atlasDir + QDir::separator() + atlasName + ".hpp"));
  writer.dev().setTextModeEnabled(true);
  QTextStream stream{&writer.dev()};

  QString nameSpace = atlasName;
  convertToIdentifier(nameSpace);
  QString headerGuard = nameSpace.toUpper();

  stream << "// This file was generated by Animera\n";
  stream << '\n';
  stream << "#ifndef ANIMERA_" << headerGuard << "_HPP\n";
  stream << "#define ANIMERA_" << headerGuard << "_HPP\n";
  stream << '\n';
  stream << "#include <cassert>\n";
  stream << "#include <cstddef>\n";
  stream << "#include <cstdint>\n";
  stream << '\n';
  stream << "namespace animera {\n";
  stream << sprite_rect_def;
  stream << sprite_rect_operators;
  stream << '\n';
  stream << "inline namespace " << nameSpace << " {\n";
  stream << '\n';
  stream << "extern const int texture_width;\n";
  stream << "extern const int texture_height;\n";
  stream << "extern const std::size_t texture_size;\n";
  stream << "extern const unsigned char texture_data[];\n";
  stream << "extern const SpriteRect sprite_rects[];\n";
  stream << '\n';
  stream << "enum class SpriteID {\n";
  stream << enumeration;
  stream << "};\n";
  stream << sprite_id_operators;
  stream << '\n';
  stream << "}\n";
  stream << '\n';
  stream << "}\n";
  stream << '\n';
  stream << "#endif\n";
  
  stream.flush();
  
  if (stream.status() == QTextStream::WriteFailed) {
    return "Error writing to file";
  }
  
  return writer.flush();
}
