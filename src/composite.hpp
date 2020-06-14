//
//  composite.hpp
//  Animera
//
//  Created by Indiana Kernick on 18/2/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_composite_hpp
#define animera_composite_hpp

#include "cel.hpp"
#include "palette span.hpp"
#include <Graphics/format.hpp>

/// Take all of the images that make up a frame and composite them together into
/// a single image
template <typename Fmt = FmtRgba>
void compositeFrame(QImage &, PaletteCSpan, const Frame &, Format, QRect);
/// Composite the overlay on top of the frame
void compositeOverlay(QImage &, const QImage &);

/// Copy an image onto another image at a position
void blitImage(QImage &, const QImage &, QPoint);

/// Copy all pixels of src that are on the mask (shifted to the position) onto
/// an image
void blitMaskImage(QImage &, const QImage &, const QImage &, QPoint);
/// Set all pixels on the mask (shifted to the position) to the color
void fillMaskImage(QImage &, const QImage &, PixelVar, QPoint);

/// Create a blank RGBA overlay image
void writeOverlay(PaletteCSpan, Format, QImage &);
/// Convert a color RGBA image to an RGBA overlay image. Colors are converted
/// to grayscale
void writeOverlay(PaletteCSpan, Format, QImage &, const QImage &);
/// Convert a color RGBA image to an RGBA overlay image using a mask.
/// Colors are converted to grayscale
void writeOverlay(PaletteCSpan, Format, QImage &, const QImage &, const QImage &);

/// Ensure that the cel is large enough to enclose the given rectangle
void growCel(Cel &, Format, QRect);
/// Shrink the cel to occupy the smallest amount of space
void shrinkCel(Cel &, QRect);
/// Sample a pixel from a cel. Returns 0 if the position is out of range
PixelVar sampleCel(const Cel &, QPoint);

#endif
