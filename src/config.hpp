//
//  config.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 26/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef config_hpp
#define config_hpp

#include "geometry.hpp"
#include <QtGui/qcolor.h>
#include <QtCore/qsize.h>
#include <QtCore/qpoint.h>
#include <QtCore/qnamespace.h>
#include "rect widget size.hpp"

// ------------------------------- global ----------------------------------- //

constexpr int       glob_scale = 2;

constexpr int operator""_px(const unsigned long long literal) {
  return static_cast<int>(literal * glob_scale);
}

// @TODO robust font system
// we might end up using
// 5x7ascii for the status bar and
// 5x7+2_var_ascii for the tooltips
// so global constants probably aren't gonna cut it
constexpr int       glob_font_width_px = 5_px;
constexpr int       glob_font_stride_px = glob_font_width_px + 1_px;
constexpr int       glob_font_accent_px = 7_px;
constexpr int       glob_font_decent_px = 0_px;
constexpr int       glob_font_px = glob_font_accent_px + glob_font_decent_px;
constexpr int       glob_font_pt = 8_px; // 8 pt for 72 dpi. 6 pt for 96 dpi

constexpr QSize     glob_min_window_size = {1280, 720};
constexpr QSize     glob_cursor_size = {8_px, 8_px};
constexpr QPoint    glob_cursor_offset = toPoint(glob_cursor_size) / 2;

inline const QColor glob_dark_shade   = {82, 82, 93};
inline const QColor glob_dark_accent  = {27, 38, 79};
inline const QColor glob_main         = {39, 70, 144};
inline const QColor glob_light_accent = {87, 108, 168};
inline const QColor glob_light_shade  = {184, 205, 215};

// -------------------------------- tools ----------------------------------- //

constexpr QSize     tool_icon_size = {24_px, 24_px};
constexpr QSize     tool_icon_padding = {1_px, 1_px};
constexpr QSize     tool_button_size = tool_icon_size + 2 * tool_icon_padding;
constexpr QPoint    tool_icon_pos = toPoint((tool_button_size - tool_icon_size)) / 2;
constexpr int       tool_select_width = tool_button_size.width() + 1_px;

inline const QColor tool_select_background = glob_dark_accent;
inline const QColor tool_base_disabled = glob_main;
inline const QColor tool_base_enabled = glob_light_accent;
inline const QColor tool_shape = glob_light_shade;

constexpr int       tool_overlay_alpha_min = 31;
constexpr int       tool_overlay_alpha = 127;
constexpr int       tool_overlay_gray = 127;
constexpr QRgb      tool_overlay_color = qRgba(
  tool_overlay_gray, tool_overlay_gray, tool_overlay_gray, tool_overlay_alpha
);

// ------------------------------- status bar ------------------------------- //

constexpr int       stat_temp_duration_ms = 4000;
constexpr int       stat_min_width = 200_px;
constexpr int       stat_padding = 1_px;
constexpr int       stat_height = glob_font_px + 2 * stat_padding;

inline const QColor stat_background = glob_dark_accent;
inline const QColor stat_text = glob_light_shade;

// --------------------------------- editor --------------------------------- //

inline const QColor edit_checker_a = {191, 191, 191};
inline const QColor edit_checker_b = {255, 255, 255};
constexpr int       edit_min_scale = 1;
constexpr int       edit_max_scale = 64;
constexpr int       edit_default_scale = 1_px;
constexpr int       edit_undo_stack = 128;

// ------------------------------ color picker ------------------------------ //

constexpr RectWidgetSize pick_svgraph_size = {
  1_px,
  1_px,
  {101_px, 101_px}
};

constexpr RectWidgetSize pick_slider_size = {
  pick_svgraph_size.padding(),
  pick_svgraph_size.border(),
  {pick_svgraph_size.content().width(), 12_px}
};

constexpr RectWidgetSize pick_label_size = {
  pick_svgraph_size.padding(),
  pick_svgraph_size.border(),
  {1 * glob_font_stride_px + 2_px, glob_font_px + 2_px}
};

constexpr RectWidgetSize pick_number_size = {
  pick_svgraph_size.padding(),
  pick_svgraph_size.border(),
  {3 * glob_font_stride_px + 1_px, glob_font_px + 2_px}
};

constexpr RectWidgetSize pick_hex_size = {
  pick_svgraph_size.padding(),
  pick_svgraph_size.border(),
  {8 * glob_font_stride_px + 6_px, glob_font_px + 2_px}
};

inline const QColor pick_label_text_color = glob_light_shade;
inline const QColor pick_svgraph_border_color = glob_light_shade;
inline const QColor pick_slider_border_color = glob_light_shade;
inline const QColor pick_primary_color = {0, 0, 0};
inline const QColor pick_secondary_color = {255, 255, 255};
inline const QColor pick_default_color = {255, 0, 0, 255};

// -------------------------------- text box -------------------------------- //

inline QColor setAlpha(QColor color, const int alpha) {
  color.setAlpha(alpha);
  return color;
}

constexpr int box_cursor_blink_interval_ms = 500;
constexpr int box_cursor_width = 1_px;
constexpr int box_text_padding = 1_px;

inline const QColor box_selection_color = setAlpha(glob_light_accent, 127);
inline const QColor box_text_color = glob_light_shade;
inline const QColor box_border_color = glob_light_shade;
inline const QColor box_cursor_color = glob_light_shade;
inline const QColor box_background_color = glob_dark_accent;

// ---------------------------------- keys ---------------------------------- //

// all tools
constexpr Qt::MouseButton mouse_primary = Qt::LeftButton;
constexpr Qt::MouseButton mouse_secondary = Qt::RightButton;
constexpr Qt::MouseButton mouse_tertiary = Qt::MiddleButton;
constexpr Qt::Key   key_primary = Qt::Key_Z;
constexpr Qt::Key   key_secondary = Qt::Key_X;
constexpr Qt::Key   key_tertiary = Qt::Key_C;

// editor
constexpr Qt::Key   key_zoom_out = Qt::Key_Q;
constexpr Qt::Key   key_zoom_in = Qt::Key_E;
constexpr Qt::Key   key_clear = Qt::Key_V;
constexpr Qt::Key   key_undo = Qt::Key_N;
constexpr Qt::Key   key_redo = Qt::Key_M;

// brush
constexpr int       brsh_min_thick = 1;
constexpr int       brsh_max_thick = 64;

// translate
constexpr Qt::Key   key_mov_up = Qt::Key_Up;
constexpr Qt::Key   key_mov_right = Qt::Key_Right;
constexpr Qt::Key   key_mov_down = Qt::Key_Down;
constexpr Qt::Key   key_mov_left = Qt::Key_Left;

// flip
constexpr Qt::Key   key_flp_on_x = Qt::Key_Right;
constexpr Qt::Key   key_flp_on_y = Qt::Key_Down;
constexpr Qt::Key   key_flp_off_x = Qt::Key_Left;
constexpr Qt::Key   key_flp_off_y = Qt::Key_Up;

// rotate
constexpr Qt::Key   key_rot_cw_a = Qt::Key_Right;
constexpr Qt::Key   key_rot_cw_b = Qt::Key_Down;
constexpr Qt::Key   key_rot_ccw_a = Qt::Key_Left;
constexpr Qt::Key   key_rot_ccw_b = Qt::Key_Up;

#endif
