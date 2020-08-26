//
//  export sprite sheet.cpp
//  Animera
//
//  Created by Indiana Kernick on 26/8/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "export sprite sheet.hpp"

#include "composite.hpp"

namespace {

struct SheetRange {
  int minor;
  int minorCount;
  int maxMinorCount;
  int major;
  int majorCount;
};

SheetRange layerRange(const SpriteNameState &state) {
  return {
    +state.frame + +state.layer * +state.frameCount,
    +state.layerCount * +state.frameCount,
    +state.layerCount * +state.frameCount,
    0,
    1
  };
}

SheetRange groupRange(const SpriteNameState &state) {
  return {
    +state.frame,
    +state.frameCount,
    +state.frameCount,
    0,
    1
  };
}

SheetRange frameRange(const SpriteNameState &state) {
  return {
    +(state.frame - state.groupBegin),
    +state.groupFrameCount,
    +state.groupFrameCount,
    0,
    1
  };
}

SheetRange frameToGroupLayerRange(const SpriteNameState &state) {
  return {
    +(state.frame - state.groupBegin),
    +state.groupFrameCount,
    +state.maxGroupFrameCount,
    +state.group + +state.layer * +state.groupCount,
    +state.groupCount * +state.layerCount
  };
}

SheetRange frameToGroupRange(const SpriteNameState &state) {
  return {
    +(state.frame - state.groupBegin),
    +state.groupFrameCount,
    +state.maxGroupFrameCount,
    +state.group,
    +state.groupCount
  };
};

SheetRange frameLayerToGroupRange(const SpriteNameState &state) {
  return {
    +state.frame - +state.groupBegin + +state.layer * +state.maxGroupFrameCount,
    +state.maxGroupFrameCount * +state.layerCount,
    +state.maxGroupFrameCount * +state.layerCount,
    +state.group,
    +state.groupCount
  };
}

SheetRange frameGroupToLayerRange(const SpriteNameState &state) {
  return {
    +state.frame,
    +state.frameCount,
    +state.frameCount,
    +state.layer,
    +state.layerCount
  };
}

QPoint columnDim(const int value, const int other) {
  return {value, other};
}

QPoint rowDim(const int value, const int other) {
  return {other, value};
}

}

template <typename Class>
auto selectFunc(const SpriteNameParams &params) {
  if (params.layerName == LayerNameMode::sheet_column) {
    if (params.groupName == GroupNameMode::sheet_row) {
      if (params.frameName == FrameNameMode::sheet_column) {
        return &Class::template funcImpl<&frameLayerToGroupRange, &columnDim>;
      } else {
        return &Class::template funcImpl<&frameGroupToLayerRange, &rowDim>;
      }
    } else {
      if (params.frameName == FrameNameMode::sheet_row) {
        return &Class::template funcImpl<&frameToGroupLayerRange, &rowDim>;
      } else {
        return &Class::template funcImpl<&layerRange, &columnDim>;
      }
    }
  } else if (params.layerName == LayerNameMode::sheet_row) {
    if (params.groupName == GroupNameMode::sheet_column) {
      if (params.frameName == FrameNameMode::sheet_row) {
        return &Class::template funcImpl<&frameLayerToGroupRange, &rowDim>;
      } else {
        return &Class::template funcImpl<&frameGroupToLayerRange, &columnDim>;
      }
    } else {
      if (params.frameName == FrameNameMode::sheet_column) {
        return &Class::template funcImpl<&frameToGroupLayerRange, &columnDim>;
      } else {
        return &Class::template funcImpl<&layerRange, &rowDim>;
      }
    }
  } else {
    if (params.groupName == GroupNameMode::sheet_column) {
      if (params.frameName == FrameNameMode::sheet_row) {
        return &Class::template funcImpl<&frameToGroupRange, &rowDim>;
      } else {
        return &Class::template funcImpl<&groupRange, &columnDim>;
      }
    } else if (params.groupName == GroupNameMode::sheet_row) {
      if (params.frameName == FrameNameMode::sheet_column) {
        return &Class::template funcImpl<&frameToGroupRange, &columnDim>;
      } else {
        return &Class::template funcImpl<&groupRange, &rowDim>;
      }
    } else {
      if (params.frameName == FrameNameMode::sheet_column) {
        return &Class::template funcImpl<&frameRange, &columnDim>;
      } else if (params.frameName == FrameNameMode::sheet_row) {
        return &Class::template funcImpl<&frameRange, &rowDim>;
      } else {
        return &Class::noSheetImpl;
      }
    }
  }
}

namespace {

QSize multiply(const QPoint pos, const QSize size) {
  return {pos.x() * size.width(), pos.y() * size.height()};
}

}

NameAppender::NameAppender(
  AtlasGenerator *generator,
  const SpriteNameParams &nameParams,
  const QSize size
) : generator{generator}, nameParams{nameParams}, size{size} {
  appendFunc = selectFunc<NameAppender>(nameParams);
}

void NameAppender::append(std::size_t &index, const SpriteNameState &state, const bool null) const {
  (this->*appendFunc)(index, state, null);
}

template <auto RangeFn, auto DimFn>
void NameAppender::funcImpl(std::size_t &index, const SpriteNameState &state, bool) const {
  const SheetRange range = RangeFn(state);
  if (range.minor == 0 && range.major == 0) {
    const QPoint count = DimFn(range.maxMinorCount, range.majorCount);
    const QSize sheetSize = multiply(count, size);
    const NameInfo info = {nameParams, state, sheetSize};
    generator->appendName(index++, info);
  }
}

void NameAppender::noSheetImpl(std::size_t &index, const SpriteNameState &state, const bool null) const {
  const NameInfo info = {nameParams, state, null ? QSize{} : size};
  generator->appendName(index++, info);
}

ImageCopier::ImageCopier(
  AtlasGenerator *generator,
  const SpriteNameParams &nameParams,
  const QSize size,
  const Format format
) : generator{generator}, size{size}, format{format} {
  copyFunc = selectFunc<ImageCopier>(nameParams);
}

Error ImageCopier::copy(std::size_t &index, const SpriteNameState &state, const QImage *image) {
  return (this->*copyFunc)(index, state, image);
}

template <auto RangeFn, auto DimFn>
Error ImageCopier::funcImpl(std::size_t &index, const SpriteNameState &state, const QImage *image) {
  const SheetRange range = RangeFn(state);
  if (range.minor == 0 && range.major == 0) {
    const QPoint count = DimFn(range.maxMinorCount, range.majorCount);
    const QSize sheetSize = multiply(count, size);
    sheetImage = {sheetSize, qimageFormat(format)};
  }
  
  const QPoint count = DimFn(range.minor, range.major);
  const QPoint pos = toPoint(multiply(count, size));
  if (image) {
    blitImage(sheetImage, *image, pos);
  } else {
    clearImage(sheetImage, {pos, size});
  }
  
  if (range.minor == range.minorCount - 1 && range.major == range.majorCount - 1) {
    return generator->copyImage(index++, sheetImage);
  }
  return {};
}

Error ImageCopier::noSheetImpl(std::size_t &index, const SpriteNameState &, const QImage *image) {
  return generator->copyImage(index++, image ? *image : QImage{});
}
