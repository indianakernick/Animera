//
//  export texture atlas.hpp
//  Animera
//
//  Created by Indiana Kernick on 15/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_export_texture_atlas_hpp
#define animera_export_texture_atlas_hpp

#include "error.hpp"
#include "export params.hpp"

class Sprite;

Error exportTextureAtlas(const ExportParams &, const std::vector<QString> &);
Error exportTextureAtlas(const ExportParams &, const Sprite &);

#endif
