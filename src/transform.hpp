//
//  transform.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 20/6/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef transform_hpp
#define transform_hpp

#include "surface.hpp"

/*

// Given points on the dst image, return points on the src image
QPoint xform(QPoint)

*/

template <typename Pixel, typename XForm>
void spatialTransform(Surface<Pixel> dst, CSurface<Pixel> src, XForm &&xform) {
  for (int y = 0; y != dst.size().height(); ++y) {
    for (int x = 0; x != dst.size().width(); ++x) {
      dst.setPixel(src.getPixel(xform(QPoint{x, y})), {x, y});
    }
  }
}

constexpr QSize rotateSize(const QSize srcSize, const int dir) {
  if ((dir & 1) == 0) {
    return srcSize;
  } else {
    return srcSize.transposed();
  }
}

template <typename Pixel>
void rotate(Surface<Pixel> dst, CSurface<Pixel> src, const int dir) {
  assert(dst.size() == rotateSize(src.size(), dir));
  const int dstWidth = dst.size().width() - 1;
  const int dstHeight = dst.size().height() - 1;
  switch (dir & 3) {
    case 0:
      return spatialTransform(dst, src, [](const QPoint dstPos) {
        return dstPos;
      });
    case 1:
      return spatialTransform(dst, src, [dstWidth](const QPoint dstPos) {
        return QPoint{dstPos.y(), dstWidth - dstPos.x()};
      });
    case 2:
      return spatialTransform(dst, src, [dstWidth, dstHeight](const QPoint dstPos) {
        return QPoint{dstWidth - dstPos.x(), dstHeight - dstPos.y()};
      });
    case 3:
      return spatialTransform(dst, src, [dstHeight](const QPoint dstPos) {
        return QPoint{dstHeight - dstPos.y(), dstPos.x()};
      });
  }
}

template <typename Pixel>
void transpose(Surface<Pixel> dst, CSurface<Pixel> src) {
  assert(dst.size() == src.size().transposed());
  spatialTransform(dst, src, [](const QPoint dstPos) {
    return QPoint{dstPos.y(), dstPos.x()};
  });
}

template <typename Pixel>
void flipVert(Surface<Pixel> dst, CSurface<Pixel> src) {
  assert(dst.size() == src.size());
  const size_t width = dst.size().width() * sizeof(Pixel);
  int srcY = dst.size().height();
  for (auto dstRow : dst.range()) {
    // @TODO Don't use pixelAddr
    std::memcpy(dstRow.begin(), src.pixelAddr({0, --srcY}), width);
  }
}

template <typename Pixel>
void flipHori(Surface<Pixel> dst, CSurface<Pixel> src) {
  assert(dst.size() == src.size());
  auto srcRowIter = src.range().begin();
  for (auto dstRow : dst.range()) {
    const Pixel *srcPixelIter = (*srcRowIter).end();
    for (Pixel &dstPixel : dstRow) {
      --srcPixelIter;
      dstPixel = *srcPixelIter;
    }
    ++srcRowIter;
  }
}

#endif
