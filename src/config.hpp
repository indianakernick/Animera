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
constexpr int       glob_font_kern_px = 1_px;
constexpr int       glob_font_stride_px = glob_font_width_px + glob_font_kern_px;
constexpr int       glob_font_accent_px = 7_px;
constexpr int       glob_font_decent_px = 0_px;
constexpr int       glob_font_px = glob_font_accent_px + glob_font_decent_px;
constexpr int       glob_font_pt = 8_px; // 8 pt for 72 dpi. 6 pt for 96 dpi

constexpr QSize     glob_window_size = {640_px, 360_px};

// ------------------------ global color scheme ----------------------------- //

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

// ----------------------------- global dimensions -------------------------- //

constexpr int       glob_scroll_width = 4_px;
constexpr int       glob_padding      = 1_px;
constexpr int       glob_text_padding = 1_px;
constexpr int       glob_border_width = 1_px;
constexpr int       glob_widget_space = 1_px;

class WidgetRect {
public:
  constexpr WidgetRect(
    const QPoint contentOffset,
    const QSize innerSize,
    const int borderWidth = glob_border_width,
    const int widgetSpace = glob_widget_space
  ) : contentOffset_{contentOffset},
      innerSize_{innerSize},
      borderWidth_{borderWidth},
      widgetSpace_{widgetSpace} {}
  
  constexpr QRect widget() const noexcept {
    return {
      QPoint{},
      innerSize_ + toSize(2 * borderWidth_ + 2 * widgetSpace_)
    };
  }
  
  constexpr QRect outer() const noexcept {
    return {
      toPoint(widgetSpace_),
      innerSize_ + toSize(2 * borderWidth_)
    };
  }
  
  constexpr QRect inner() const noexcept {
    return {
      toPoint(widgetSpace_ + borderWidth_),
      innerSize_
    };
  }
  
  constexpr QPoint contentPos() const noexcept {
    return toPoint(widgetSpace_ + borderWidth_) + contentOffset_;
  }
  
  constexpr int borderWidth() const noexcept {
    return borderWidth_;
  }
  
  constexpr int widgetSpace() const noexcept {
    return widgetSpace_;
  }

private:
  QPoint contentOffset_;
  QSize innerSize_;
  int borderWidth_;
  int widgetSpace_;
};

// -------------------------------- tools ----------------------------------- //

constexpr QSize     tool_icon_size = {24_px, 24_px};
constexpr QSize     tool_icon_padding = {1_px, 1_px};
constexpr QSize     tool_button_size = tool_icon_size + 2 * tool_icon_padding;
constexpr QPoint    tool_icon_pos = toPoint(tool_icon_padding);
constexpr int       tool_select_width = tool_button_size.width() + 2 * glob_padding;

inline const QColor tool_select_background = glob_main;
inline const QColor tool_base_disabled = glob_light_1;
inline const QColor tool_base_enabled = glob_light_2;
inline const QColor tool_shape_disabled = glob_light_2;
inline const QColor tool_shape_enabled = glob_light_1;

constexpr int       tool_overlay_alpha_min = 31;
constexpr int       tool_overlay_alpha = 127;
constexpr int       tool_overlay_gray = 127;
constexpr QRgb      tool_overlay_color = qRgba(
  tool_overlay_gray, tool_overlay_gray, tool_overlay_gray, tool_overlay_alpha
);

// ------------------------------- status bar ------------------------------- //

constexpr int       stat_temp_duration_ms = 2500;
constexpr int       stat_min_width = 200_px;
constexpr int       stat_height = glob_font_px + 2 * (glob_text_padding + glob_padding);

inline const QColor stat_background = glob_main;

// --------------------------------- editor --------------------------------- //

constexpr int       edit_min_scale = 1;
constexpr int       edit_max_scale = 64;
constexpr int       edit_undo_stack = 128;

inline const QRgb edit_checker_a = qRgb(191, 191, 191);
inline const QRgb edit_checker_b = qRgb(255, 255, 255);

// ------------------------------ color picker ------------------------------ //

constexpr QSize textBoxSize(const int chars, const int offsetX) {
  return {
    chars * glob_font_stride_px - glob_font_kern_px + 2 * glob_text_padding + offsetX,
    glob_font_px + 2 * glob_text_padding
  };
}

constexpr WidgetRect textBoxRect(const int chars, const int offsetX) {
  return {
    {glob_text_padding + offsetX, glob_text_padding},
    textBoxSize(chars, offsetX)
  };
}

constexpr WidgetRect boxRect(const int width, const int height) {
  return {{}, {width, height}};
}

constexpr WidgetRect pick_svgraph_rect = boxRect(101_px, 101_px);
constexpr WidgetRect pick_slider_rect = boxRect(pick_svgraph_rect.inner().width(), 12_px);
constexpr WidgetRect pick_label_rect = textBoxRect(1, 1_px);
constexpr WidgetRect pick_number_rect = textBoxRect(3, 0_px);
constexpr WidgetRect pick_hex_rect = {
  QPoint{glob_text_padding + 2_px, glob_text_padding},
  textBoxSize(8, 2_px + 3_px)
};
constexpr WidgetRect pick_name_rect = {
  toPoint(glob_text_padding),
  QSize{pick_svgraph_rect.inner().width(), glob_font_px + 2 * glob_text_padding}
};

inline const QColor pick_primary_color = {0, 0, 0};
inline const QColor pick_secondary_color = {255, 255, 255};
inline const QColor pick_default_color = {255, 0, 0, 255};

constexpr int       pick_alpha_tiles = 2;

// -------------------------------- text box -------------------------------- //

inline QColor setAlpha(QColor color, const int alpha) {
  color.setAlpha(alpha);
  return color;
}

constexpr int       box_cursor_blink_interval_ms = 500;
constexpr int       box_cursor_width = glob_font_kern_px;

inline const QColor box_selection_color = setAlpha(glob_light_1, 127);
inline const QColor box_background_color = glob_dark_1;

// ------------------------------- tool colors ------------------------------ //

constexpr WidgetRect tool_color_rect = {{}, {24_px, 12_px}, 1_px, 2_px};
constexpr WidgetRect active_color_rect = {{}, {24_px, 12_px}, 2_px, 1_px};
constexpr int        tool_color_tiles = 2;
constexpr WidgetRect tool_colors_rect = {
  {}, {pick_svgraph_rect.inner().width(), 12_px}, 0_px, 3_px
};

// ------------------------------ palette colors ---------------------------- //

constexpr int       pal_colors = 256;
constexpr int       pal_width = 8;
constexpr int       pal_height = pal_colors / pal_width;
constexpr int       pal_tile_size = 13_px;

// --------------------------------- timeline ------------------------------- //

inline const QColor cell_icon_color = glob_light_2;
inline const QColor cell_curr_color = glob_light_1;

constexpr int       cell_icon_pad = 1_px;
constexpr int       cell_icon_size = 8_px;
constexpr int       cell_icon_step = 2 * cell_icon_pad + cell_icon_size + glob_border_width;
constexpr int       cell_height = cell_icon_step;
constexpr int       cell_border_offset = cell_icon_pad + glob_border_width;
constexpr int       frame_incr = 5;

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
constexpr int       brsh_min_radius = 0;
constexpr int       brsh_max_radius = 64;

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
