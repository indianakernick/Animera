//
//  flood fill.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 25/5/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef flood_fill_hpp
#define flood_fill_hpp

/*

struct Manip {
  Manip(const Manip &);
  // start with this pixel. Returns whether we should continue
  bool start(QPoint);
  // size of the underlying surface
  QSize size() const;
  // should this pixel be filled?
  bool shouldSet(QPoint) const; // !filled
  // fill a pixel
  void set(QPoint);
};

*/

namespace detail {

QPoint up(const QPoint p) {
  return {p.x(), p.y() - 1};
}

QPoint right(const QPoint p) {
  return {p.x() + 1, p.y()};
}

QPoint down(const QPoint p) {
  return {p.x(), p.y() + 1};
}

QPoint left(const QPoint p) {
  return {p.x() - 1, p.y()};
}

// Flood Fill algorithm by Adam Milazzo
// http://www.adammil.net/blog/v126_A_More_Efficient_Flood_Fill.html

template <typename Manip>
void floodFillStart(Manip, QPoint, QSize);

template <typename Manip>
void floodFillCore(Manip manip, QPoint pos, const QSize size) {
  int lastRowLength = 0;
  do {
    int rowLength = 0;
    QPoint start = pos;
    if (lastRowLength != 0 && !manip.shouldSet(pos)) {
      do {
        if (--lastRowLength == 0) return;
        pos = right(pos);
      } while (!manip.shouldSet(pos));
    } else {
      while (pos.x() != 0 && manip.shouldSet(left(pos))) {
        pos = left(pos);
        manip.set(pos);
        if (pos.y() != 0 && manip.shouldSet(up(pos))) {
          floodFillStart(manip, up(pos), size);
        }
        ++rowLength;
        ++lastRowLength;
      }
    }
    while (start.x() < size.width() && manip.shouldSet(start)) {
      manip.set(start);
      start = right(start);
      ++rowLength;
    }
    if (rowLength < lastRowLength) {
      const int endX = pos.x() + lastRowLength;
      while (++start.rx() < endX) {
        if (manip.shouldSet(start)) {
          floodFillCore(manip, start, size);
        }
      }
    } else if (rowLength > lastRowLength && pos.y() != 0) {
      QPoint above = up({pos.x() + lastRowLength, pos.y()});
      while (++above.rx() < start.x()) {
        if (manip.shouldSet(above)) {
          floodFillStart(manip, above, size);
        }
      }
    }
    lastRowLength = rowLength;
    pos = down(pos);
  } while (lastRowLength != 0 && pos.y() < size.height());
}

template <typename Manip>
void floodFillStart(Manip manip, QPoint pos, const QSize size) {
  while (true) {
    const QPoint startPos = pos;
    while (pos.y() != 0 && manip.shouldSet(up(pos))) pos = up(pos);
    while (pos.x() != 0 && manip.shouldSet(left(pos))) pos = left(pos);
    if (pos == startPos) break;
  }
  floodFillCore(manip, pos, size);
}

}

template <typename Manip>
bool floodFill(Manip manip, const QPoint pos) {
  if (!manip.start(pos)) return false;
  detail::floodFillStart(manip, pos, manip.size());
  return true;
}

#endif
