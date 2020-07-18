//
//  cel array.hpp
//  Animera
//
//  Created by Indiana Kernick on 25/7/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_cel_array_hpp
#define animera_cel_array_hpp

#include "cel.hpp"
#include <tcb/span.hpp>

struct Cel {
  CelImagePtr cel;
  FrameIdx len;
};

struct Layer {
  std::vector<Cel> cels;
  std::string name;
  bool visible = true;
};

class CelIterator {
public:
  CelIterator(tcb::span<const Cel>, FrameIdx);
  
  const CelImage *img() const;
  bool incr();

private:
  tcb::span<const Cel>::iterator iter;
  FrameIdx idx;
};

/// Combine consecutive null cels into a single cel
void optimizeCelArray(std::vector<Cel> &);

/// Get a mutable image
CelImage *getImage(std::vector<Cel> &, FrameIdx);

/// Get a constant image
const CelImage *getImage(tcb::span<const Cel>, FrameIdx);

/// Insert a new cel after the index
void insertCelFrame(std::vector<Cel> &, FrameIdx);

/// Replace a cel with a new cel
void replaceCelFrame(std::vector<Cel> &, FrameIdx, bool);

/// Extend the cel at the index and shrink the following cel
/// The cel will not be extended past the end
void extendCel(std::vector<Cel> &, FrameIdx);

/// Split the cel at the index into two
/// The index becomes the first frame of the second cel
void splitCel(std::vector<Cel> &, FrameIdx);

/// Replace part of an array with another array
void replaceCelArray(std::vector<Cel> &, FrameIdx, std::vector<Cel> &);

/// Make a copy of part of the cel array
std::vector<Cel> extractCelArray(tcb::span<const Cel>, FrameIdx, FrameIdx);

/// Make a truncated copy of the cel array
std::vector<Cel> truncateCopyCelArray(tcb::span<const Cel>, FrameIdx);

/// Remove a cel
void removeCelFrame(std::vector<Cel> &, FrameIdx);

/// Replace the cels with a single null cel
void clearCelArray(std::vector<Cel> &, FrameIdx);

#endif
