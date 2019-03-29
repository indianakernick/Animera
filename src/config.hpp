//
//  config.hpp
//  Pixel 2
//
//  Created by Indi Kernick on 26/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef config_hpp
#define config_hpp

#include "utils.hpp"
#include <QtGui/qcolor.h>
#include <QtCore/qsize.h>
#include <QtCore/qpoint.h>
#include <QtCore/qnamespace.h>

// ------------------------------- global ------------------------------------//

constexpr int       glob_scale = 2;
constexpr int       glob_text = 9 * glob_scale;
constexpr QSize     glob_min_window_size = {1280, 720};
constexpr int       glob_font_pt = glob_scale * 16; // 16 pt for 72 dpi. 12 pt for 96 dpi

inline const QColor glob_dark_shade   = {82, 82, 93};
inline const QColor glob_dark_accent  = {27, 38, 79};
inline const QColor glob_main         = {39, 70, 144};
inline const QColor glob_light_accent = {87, 108, 168};
inline const QColor glob_light_shade  = {184, 205, 215};

// -------------------------------- tools ------------------------------------//

constexpr QSize     tool_icon_size = QSize{24, 24} * glob_scale;
constexpr QSize     tool_icon_padding = QSize{1, 1} * glob_scale;
constexpr QSize     tool_button_size = tool_icon_size + 2 * tool_icon_padding;
constexpr QPoint    tool_icon_pos = toPoint((tool_button_size - tool_icon_size)) / 2;
constexpr int       tool_select_width = tool_button_size.width() + 1 * glob_scale;

inline const QColor tool_select_background = glob_dark_accent;
inline const QColor tool_base_disabled = glob_main;
inline const QColor tool_base_enabled = glob_light_accent;
inline const QColor tool_shape = glob_light_shade;

// ------------------------------- status bar --------------------------------//

constexpr int       stat_temp_duration_ms = 5000;
constexpr int       stat_min_width = 200 * glob_scale;
constexpr int       stat_padding = 1 * glob_scale;
constexpr int       stat_height = glob_text + 2 * stat_padding;

inline const QColor stat_background = glob_dark_accent;
inline const QColor stat_text = glob_light_shade;

// --------------------------------- editor ----------------------------------//

inline const QColor edit_checker_a = {191, 191, 191};
inline const QColor edit_checker_b = {255, 255, 255};
constexpr int       edit_min_scale = 1;
constexpr int       edit_max_scale = 64;
constexpr int       edit_default_scale = 2;
constexpr QSize     edit_cursor_size = QSize{8, 8} * glob_scale;
constexpr QPoint    edit_cursor_offset = toPoint(edit_cursor_size) / 2;

// ----------------------------------- keys ----------------------------------//

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

// brush
constexpr Qt::Key   key_clear = Qt::Key_V;
constexpr int       brsh_min_thick = 1;
constexpr int       brsh_max_thick = 64;

// select
constexpr Qt::Key   key_toggle_copy_paste = Qt::Key_P;

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
