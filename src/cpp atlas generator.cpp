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

[[nodiscard]] inline TextureInfo getTextureInfo(SpriteID = SpriteID::null_) noexcept {
  return {texture_data, texture_size, texture_pitch, texture_width, texture_height};
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

constexpr char texture_info_def[] = R"(
struct TextureInfo {
  const unsigned char *data;
  std::size_t size;
  std::size_t pitch;
  int width;
  int height;
};
)";

constexpr char zlib_decompress_lib[] = R"(
/*
* tinflate - tiny inflate
*
* Copyright (c) 2003-2019 Joergen Ibsen
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must
*      not claim that you wrote the original software. If you use this
*      software in a product, an acknowledgment in the product
*      documentation would be appreciated but is not required.
*
*   2. Altered source versions must be plainly marked as such, and must
*      not be misrepresented as being the original software.
*
*   3. This notice may not be removed or altered from any source
*      distribution.
*/

// Version 1.1.0 with minor changes

#include <memory>
#include <assert.h>
#include <limits.h>

namespace {

#if defined(UINT_MAX) && (UINT_MAX) < 0xFFFFFFFFUL
#  error "tinf requires unsigned int to be at least 32-bit"
#endif

typedef enum {
  TINF_OK         = 0,  /**< Success */
  TINF_DATA_ERROR = -3, /**< Input error */
  TINF_BUF_ERROR  = -5  /**< Not enough room for output */
} tinf_error_code;

/* ------------------------------ *
 * -- internal data structures -- *
 * ------------------------------ */

struct tinf_tree {
  unsigned short table[16]; /* table of code length counts */
  unsigned short trans[288]; /* code -> symbol translation table */
};

struct tinf_data {
  const unsigned char *source;
  unsigned int tag;
  int bitcount;

  unsigned char *dest;
  unsigned int *destLen;

  struct tinf_tree ltree; /* literal/length tree */
  struct tinf_tree dtree; /* distance tree */
};

/* ----------------------- *
 * -- utility functions -- *
 * ----------------------- */

unsigned int read_le16(const unsigned char *p)
{
  return ((unsigned int) p[0])
       | ((unsigned int) p[1] << 8);
}

/* build the fixed huffman trees */
void tinf_build_fixed_trees(struct tinf_tree *lt, struct tinf_tree *dt)
{
  int i;

  /* build fixed length tree */
  for (i = 0; i < 16; ++i) {
    lt->table[i] = 0;
  }

  lt->table[7] = 24;
  lt->table[8] = 152;
  lt->table[9] = 112;

  for (i = 0; i < 24; ++i) {
    lt->trans[i] = 256 + i;
  }
  for (i = 0; i < 144; ++i) {
    lt->trans[24 + i] = i;
  }
  for (i = 0; i < 8; ++i) {
    lt->trans[24 + 144 + i] = 280 + i;
  }
  for (i = 0; i < 112; ++i) {
    lt->trans[24 + 144 + 8 + i] = 144 + i;
  }

  /* build fixed distance tree */
  for (i = 0; i < 16; ++i) {
    dt->table[i] = 0;
  }

  dt->table[5] = 32;

  for (i = 0; i < 32; ++i) {
    dt->trans[i] = i;
  }
}

/* given an array of code lengths, build a tree */
int tinf_build_tree(struct tinf_tree *t, const unsigned char *lengths,
                           unsigned int num)
{
  unsigned short offs[16];
  unsigned int i, sum;

  assert(num < 288);

  /* clear code length count table */
  for (i = 0; i < 16; ++i) {
    t->table[i] = 0;
  }

  /* scan symbol lengths, and sum code length counts */
  for (i = 0; i < num; ++i) {
    t->table[lengths[i]]++;
  }

  t->table[0] = 0;

  /* compute offset table for distribution sort */
  for (sum = 0, i = 0; i < 16; ++i) {
    offs[i] = sum;
    sum += t->table[i];
  }

  /* create code->symbol translation table (symbols sorted by code) */
  for (i = 0; i < num; ++i) {
    if (lengths[i]) {
      t->trans[offs[lengths[i]]++] = i;
    }
  }

  return TINF_OK;
}

/* ---------------------- *
 * -- decode functions -- *
 * ---------------------- */

void tinf_refill(struct tinf_data *d, int num)
{
  assert(num >= 0 && num <= 32);

  /* read bytes until at least num bits available */
  while (d->bitcount < num) {
    d->tag |= (unsigned int) *d->source++ << d->bitcount;
    d->bitcount += 8;
  }

  assert(d->bitcount <= 32);
}

unsigned int tinf_getbits_no_refill(struct tinf_data *d, int num)
{
  unsigned int bits;

  assert(num >= 0 && num <= d->bitcount);

  /* get bits from tag */
  bits = d->tag & ((1UL << num) - 1);

  /* remove bits from tag */
  d->tag >>= num;
  d->bitcount -= num;

  return bits;
}

/* get num bits from source stream */
unsigned int tinf_getbits(struct tinf_data *d, int num)
{
  tinf_refill(d, num);
  return tinf_getbits_no_refill(d, num);
}

/* read a num bit value from stream and add base */
unsigned int tinf_getbits_base(struct tinf_data *d, int num, int base)
{
  return base + (num ? tinf_getbits(d, num) : 0);
}

/* given a data stream and a tree, decode a symbol */
int tinf_decode_symbol(struct tinf_data *d, const struct tinf_tree *t)
{
  int sum = 0, cur = 0, len = 0;

  /* get more bits while code value is above sum */
  do {
    cur = 2 * cur + tinf_getbits(d, 1);

    ++len;

    assert(len <= 15);

    sum += t->table[len];
    cur -= t->table[len];
  } while (cur >= 0);

  assert(sum + cur >= 0 && sum + cur < 288);

  return t->trans[sum + cur];
}

/* given a data stream, decode dynamic trees from it */
int tinf_decode_trees(struct tinf_data *d, struct tinf_tree *lt,
                             struct tinf_tree *dt)
{
  unsigned char lengths[288 + 32];

  /* special ordering of code length codes */
  static const unsigned char clcidx[19] = {
    16, 17, 18, 0,  8, 7,  9, 6, 10, 5,
    11,  4, 12, 3, 13, 2, 14, 1, 15
  };

  unsigned int hlit, hdist, hclen;
  unsigned int i, num, length;
  int res;

  /* get 5 bits HLIT (257-286) */
  hlit = tinf_getbits_base(d, 5, 257);

  /* get 5 bits HDIST (1-32) */
  hdist = tinf_getbits_base(d, 5, 1);

  /* get 4 bits HCLEN (4-19) */
  hclen = tinf_getbits_base(d, 4, 4);

  for (i = 0; i < 19; ++i) {
    lengths[i] = 0;
  }

  /* read code lengths for code length alphabet */
  for (i = 0; i < hclen; ++i) {
    /* get 3 bits code length (0-7) */
    unsigned int clen = tinf_getbits(d, 3);

    lengths[clcidx[i]] = clen;
  }

  /* build code length tree (in literal/length tree to save space) */
  res = tinf_build_tree(lt, lengths, 19);

  if (res != TINF_OK) {
    return res;
  }

  /* decode code lengths for the dynamic trees */
  for (num = 0; num < hlit + hdist; ) {
    int sym = tinf_decode_symbol(d, lt);

    switch (sym) {
    case 16:
      /* copy previous code length 3-6 times (read 2 bits) */
      sym = lengths[num - 1];
      length = tinf_getbits_base(d, 2, 3);
      break;
    case 17:
      /* repeat code length 0 for 3-10 times (read 3 bits) */
      sym = 0;
      length = tinf_getbits_base(d, 3, 3);
      break;
    case 18:
      /* repeat code length 0 for 11-138 times (read 7 bits) */
      sym = 0;
      length = tinf_getbits_base(d, 7, 11);
      break;
    default:
      /* values 0-15 represent the actual code lengths */
      length = 1;
      break;
    }

    while (length--) {
      lengths[num++] = sym;
    }
  }

  /* build dynamic trees */
  res = tinf_build_tree(lt, lengths, hlit);

  if (res != TINF_OK) {
    return res;
  }

  res = tinf_build_tree(dt, lengths + hlit, hdist);

  if (res != TINF_OK) {
    return res;
  }

  return TINF_OK;
}

/* ----------------------------- *
 * -- block inflate functions -- *
 * ----------------------------- */

/* given a stream and two trees, inflate a block of data */
int tinf_inflate_block_data(struct tinf_data *d, struct tinf_tree *lt,
                                   struct tinf_tree *dt)
{
  /* extra bits and base tables for length codes */
  static const unsigned char length_bits[30] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
    1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
    4, 4, 4, 4, 5, 5, 5, 5, 0, 127
  };

  static const unsigned short length_base[30] = {
     3,  4,  5,   6,   7,   8,   9,  10,  11,  13,
    15, 17, 19,  23,  27,  31,  35,  43,  51,  59,
    67, 83, 99, 115, 131, 163, 195, 227, 258,   0
  };

  /* extra bits and base tables for distance codes */
  static const unsigned char dist_bits[30] = {
    0, 0,  0,  0,  1,  1,  2,  2,  3,  3,
    4, 4,  5,  5,  6,  6,  7,  7,  8,  8,
    9, 9, 10, 10, 11, 11, 12, 12, 13, 13
  };

  static const unsigned short dist_base[30] = {
       1,    2,    3,    4,    5,    7,    9,    13,    17,    25,
      33,   49,   65,   97,  129,  193,  257,   385,   513,   769,
    1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
  };

  /* remember current output position */
  unsigned char *start = d->dest;

  for (;;) {
    int sym = tinf_decode_symbol(d, lt);

    /* check for end of block */
    if (sym == 256) {
      *d->destLen += d->dest - start;
      return TINF_OK;
    }

    if (sym < 256) {
      *d->dest++ = sym;
    }
    else {
      int length, dist, offs;
      int i;

      sym -= 257;

      /* possibly get more bits from length code */
      length = tinf_getbits_base(d, length_bits[sym],
                              length_base[sym]);

      dist = tinf_decode_symbol(d, dt);

      /* possibly get more bits from distance code */
      offs = tinf_getbits_base(d, dist_bits[dist],
                            dist_base[dist]);

      /* copy match */
      for (i = 0; i < length; ++i) {
        d->dest[i] = d->dest[i - offs];
      }

      d->dest += length;
    }
  }
}

/* inflate an uncompressed block of data */
int tinf_inflate_uncompressed_block(struct tinf_data *d)
{
  unsigned int length, invlength;
  unsigned int i;

  /* get length */
  length = read_le16(d->source);

  /* get one's complement of length */
  invlength = read_le16(d->source + 2);

  /* check length */
  if (length != (~invlength & 0x0000FFFF)) {
    return TINF_DATA_ERROR;
  }

  d->source += 4;

  /* copy block */
  for (i = length; i; --i) {
    *d->dest++ = *d->source++;
  }

  /* make sure we start next block on a byte boundary */
  d->tag = 0;
  d->bitcount = 0;

  *d->destLen += length;

  return TINF_OK;
}

/* inflate a block of data compressed with fixed huffman trees */
int tinf_inflate_fixed_block(struct tinf_data *d)
{
  /* build fixed huffman trees */
  tinf_build_fixed_trees(&d->ltree, &d->dtree);

  /* decode block using fixed trees */
  return tinf_inflate_block_data(d, &d->ltree, &d->dtree);
}

/* inflate a block of data compressed with dynamic huffman trees */
int tinf_inflate_dynamic_block(struct tinf_data *d)
{
  /* decode trees from stream */
  int res = tinf_decode_trees(d, &d->ltree, &d->dtree);

  if (res != TINF_OK) {
    return res;
  }

  /* decode block using decoded trees */
  return tinf_inflate_block_data(d, &d->ltree, &d->dtree);
}

/* ---------------------- *
 * -- public functions -- *
 * ---------------------- */

/* inflate stream from source to dest */
int tinf_uncompress(void *dest, unsigned int *destLen,
                    const void *source, unsigned int sourceLen)
{
  struct tinf_data d;
  int bfinal;

  /* initialise data */
  d.source = (const unsigned char *) source;
  d.tag = 0;
  d.bitcount = 0;

  d.dest = (unsigned char *) dest;
  d.destLen = destLen;

  *destLen = 0;

  do {
    unsigned int btype;
    int res;

    /* read final block flag */
    bfinal = tinf_getbits(&d, 1);

    /* read block type (2 bits) */
    btype = tinf_getbits(&d, 2);

    /* decompress block */
    switch (btype) {
    case 0:
      /* decompress uncompressed block */
      res = tinf_inflate_uncompressed_block(&d);
      break;
    case 1:
      /* decompress block with fixed huffman trees */
      res = tinf_inflate_fixed_block(&d);
      break;
    case 2:
      /* decompress block with dynamic huffman trees */
      res = tinf_inflate_dynamic_block(&d);
      break;
    default:
      res = TINF_DATA_ERROR;
      break;
    }

    if (res != TINF_OK) {
      return res;
    }
  } while (!bfinal);

  return TINF_OK;
}

}
)";

constexpr char zlib_decompress_func[] = R"(
std::unique_ptr<const unsigned char []> decompressTexture(const TextureInfo &info) noexcept {
  unsigned dstLen = info.pitch * info.height;
  auto *dst = new unsigned char[dstLen];
  const int ret = tinf_uncompress(dst, &dstLen, info.data, info.size);
  if (ret == TINF_OK) {
    return std::unique_ptr<const unsigned char []>{dst};
  } else {
    return nullptr;
  }
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

CppAtlasGenerator::CppAtlasGenerator(const DataFormat format, const bool withInflate)
  : packer{format}, withInflate{withInflate} {}

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
  insertName("count_");
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
  constexpr std::size_t bytes_per_line = 80 / 5;
  constexpr char hex_chars[] = "0123456789ABCDEF";
  const char zero[] = "0,";
  char hexOne[] = "0x0,";
  char hexTwo[] = "0x00,";

  for (std::size_t i = 0; i != size; ++i) {
    if (i % bytes_per_line == 0) {
      if (!dev.putChar('\n')) {
        return dev.errorString();
      }
    }
    
    static_assert(CHAR_BIT == 8);
    const unsigned char byte = data[i];
    const unsigned char hi = byte >> 4;
    const unsigned char lo = byte & 15;
    
    if (byte == 0) {
      if (dev.write(zero, 2) != 2) {
        return dev.errorString();
      }
    } else if (hi == 0) {
      hexOne[2] = hex_chars[byte];
      if (dev.write(hexOne, 4) != 4) {
        return dev.errorString();
      }
    } else {
      hexTwo[2] = hex_chars[hi];
      hexTwo[3] = hex_chars[lo];
      if (dev.write(hexTwo, 5) != 5) {
        return dev.errorString();
      }
    }
  }
  
  return {};
}

Error CppAtlasGenerator::writeCpp() {
  QBuffer textureBuffer;
  textureBuffer.open(QIODevice::ReadWrite);
  TRY(packer.write(textureBuffer));
  
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
  stream << "extern const std::size_t texture_pitch = " << packer.pitch() << ";\n";
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
  if (withInflate) {
    stream << zlib_decompress_lib;
    stream << '\n';
    stream << "namespace animera {\n";
    stream << texture_info_def;
    stream << zlib_decompress_func;
    stream << '\n';
    stream << "}\n";
  }
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
  if (withInflate) {
    stream << "#include <memory>\n";
  }
  stream << "#include <cassert>\n";
  stream << "#include <cstddef>\n";
  stream << "#include <cstdint>\n";
  stream << '\n';
  stream << "namespace animera {\n";
  stream << sprite_rect_def;
  stream << sprite_rect_operators;
  stream << texture_info_def;
  if (withInflate) {
    stream << '\n';
    stream << "std::unique_ptr<const unsigned char []> decompressTexture(const TextureInfo &) noexcept;\n";
  }
  stream << '\n';
  stream << "inline namespace " << nameSpace << " {\n";
  stream << '\n';
  stream << "extern const int texture_width;\n";
  stream << "extern const int texture_height;\n";
  stream << "extern const std::size_t texture_pitch;\n";
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
