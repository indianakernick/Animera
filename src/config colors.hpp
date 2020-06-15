//
//  config colors.hpp
//  Animera
//
//  Created by Indiana Kernick on 15/6/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_config_colors_hpp
#define animera_config_colors_hpp

#include "math.hpp"
#include "int range.hpp"
#include <QtGui/qcolor.h>

inline QColor setAlpha(QColor color, const int alpha) {
  color.setAlpha(alpha);
  return color;
}

// --------------------------- global color scheme -------------------------- //

// https://coolors.co/49495b-2f395f-274690-5569a2-b8cdd7
inline const QColor glob_dark_2  = {73, 73, 91};
inline const QColor glob_dark_1  = {47, 57, 95};
inline const QColor glob_main    = {39, 70, 144};
inline const QColor glob_light_1 = {85, 105, 162};
inline const QColor glob_light_2 = {184, 205, 215};

inline const QColor glob_text_color = glob_light_2;
inline const QColor glob_border_color = glob_light_2;
inline const QColor glob_back_color = glob_dark_2;

inline const QColor glob_scroll_corner = glob_dark_1;
inline const QColor glob_scroll_back = glob_dark_1;
inline const QColor glob_scroll_handle = glob_light_1;

// ------------------------------- tool select ------------------------------ //

inline const QColor tool_select_background = glob_main;
inline const QColor tool_base_disabled = glob_light_1;
inline const QColor tool_base_enabled = glob_light_2;
inline const QColor tool_shape_disabled = glob_light_2;
inline const QColor tool_shape_enabled = glob_light_1;

constexpr IntRange  tool_overlay_gray = {32, 223, 127};
constexpr IntRange  tool_overlay_alpha = {32, 191, 127};
constexpr QRgb      tool_overlay_color = qRgba( // premul
  tool_overlay_gray.def,
  tool_overlay_gray.def,
  tool_overlay_gray.def,
  tool_overlay_alpha.def
);

constexpr int scaleOverlay(const IntRange range, const int value) {
  return scale(value, 0, 255, range.min, range.max);
}
constexpr int scaleOverlayGray(const int gray) {
  return scaleOverlay(tool_overlay_gray, gray);
}
constexpr int scaleOverlayAlpha(const int alpha) {
  return scaleOverlay(tool_overlay_alpha, alpha);
}

// ------------------------------- status bar ------------------------------- //

inline const QColor  stat_background = glob_main;

// --------------------------------- editor --------------------------------- //

inline const QRgb   edit_checker_a = qRgb(191, 191, 191);
inline const QRgb   edit_checker_b = qRgb(255, 255, 255);

// ------------------------------ color picker ------------------------------ //

inline const QColor pick_primary_color = {0, 0, 0};
inline const QColor pick_secondary_color = {255, 255, 255};
inline const QColor pick_default_color = {255, 0, 0, 255};
constexpr int       pick_default_gray = 255;

// -------------------------------- text box -------------------------------- //

inline const QColor box_selection_color = setAlpha(glob_light_1, 127);
inline const QColor box_background_color = glob_dark_1;

// -------------------------------- timeline -------------------------------- //

inline const QColor cel_icon_color = glob_light_2;
inline const QColor cel_pos_color = glob_light_1;
inline const QColor cel_select_color = glob_dark_1;
inline const QColor cel_select_border_color = glob_dark_2;

#endif
