//
//  composite.hpp
//  Animera
//
//  Created by Indi Kernick on 18/2/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef composite_hpp
#define composite_hpp

#include <span>
#include "cell.hpp"
#include "palette span.hpp"
#include <Graphics/format.hpp>

/// Take all of the images that make up a frame and composite them together into
/// a single image
template <typename PxFmt = gfx::ARGB>
void compositeFrame(QImage &, PaletteCSpan, const Frame &, Format);
/// Composite the overlay on top of the frame
void compositeOverlay(QImage &, const QImage &);

/// Copy an image onto another image at a position
void blitImage(QImage &, const QImage &, QPoint);
/// Copy part of an image into a new image
QImage blitImage(const QImage &, QRect);

/// Copy all pixels of src that are on the mask (shifted to the position) onto
/// onto an image
void blitMaskImage(QImage &, const QImage &, const QImage &, QPoint);
/// Copy all pixels of src that are on the mask (shifted to the position) onto a
/// new image
QImage blitMaskImage(const QImage &, const QImage &, QPoint);

/// Convert a color RGBA image to an RGBA overlay image. Colors are converted
/// to grayscale
void writeOverlay(PaletteCSpan, Format, QImage &, const QImage &);
/// Convert a color RGBA image to an RGBA overlay image using a mask.
/// Colors are converted to grayscale
void writeOverlay(PaletteCSpan, Format, QImage &, const QImage &, const QImage &);

/// Ensure that the cell is large enough to enclose the given rectangle
void growCell(Cell &, Format, QRect);
/// Shrink the cell to occupy the smallest amount of space
void optimizeCell(Cell &);

#endif
