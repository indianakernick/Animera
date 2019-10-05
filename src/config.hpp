//
//  config.hpp
//  Animera
//
//  Created by Indi Kernick on 26/3/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef config_hpp
#define config_hpp

#include "geometry.hpp"
#include <QtGui/qcolor.h>
#include "widget rect.hpp"
#include <QtCore/qnamespace.h>

// ------------------------------- global ----------------------------------- //

constexpr int       glob_scale = 2;

constexpr int operator""_px(const unsigned long long literal) {
  return static_cast<int>(literal * glob_scale);
}

// TODO: robust font system
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
constexpr int       glob_margin       = 1_px;
constexpr int       glob_text_margin  = 1_px;
constexpr int       glob_border_width = 1_px;

constexpr WidgetRect basicRect(
  const QPoint pos,
  const QSize innerSize,
  const int borderWidth = glob_border_width,
  const int margin = glob_margin
) {
  const QRect widget = {
    QPoint{},
    innerSize + toSize(2 * borderWidth + 2 * margin)
  };
  const QRect outer = {
    toPoint(margin),
    innerSize + toSize(2 * borderWidth)
  };
  const QRect inner = {
    toPoint(margin + borderWidth),
    innerSize
  };
  return {widget, outer, inner, inner.topLeft() + pos};
}

constexpr int textBoxWidth(const int chars) {
  return chars * glob_font_stride_px - glob_font_kern_px + 2 * glob_text_margin;
}

constexpr int textBoxHeight(const int lines) {
  return lines * (glob_font_px + glob_text_margin) + glob_text_margin;
}

constexpr QSize textBoxSize(const int chars, const int offsetX = 0) {
  return {
    textBoxWidth(chars) + offsetX,
    textBoxHeight(1)
  };
}

constexpr WidgetRect textBoxRect(const int chars, const int offsetX = 0) {
  return basicRect(
    {glob_text_margin + offsetX, glob_text_margin},
    textBoxSize(chars, offsetX)
  );
}

constexpr WidgetRect boxRect(const int width, const int height) {
  return basicRect({}, {width, height});
}

struct TextIconRects {
  WidgetRect text;
  WidgetRect icon;
};

constexpr TextIconRects textBoxIconRect(const int chars) {
  const QSize iconInnerSize = textBoxSize(1);
  const QRect iconWidget = {
    QPoint{},
    iconInnerSize + QSize{glob_border_width + glob_margin, 2 * glob_border_width + 2 * glob_margin}
  };
  const QRect iconOuter = {
    {0, glob_margin},
    iconInnerSize + QSize{glob_border_width, 2 * glob_border_width}
  };
  const QRect iconInner = {
    {0, glob_margin + glob_border_width},
    iconInnerSize
  };
  const QPoint iconPos = {
    glob_text_margin, glob_text_margin + glob_border_width + glob_margin
  };
  const WidgetRect text = textBoxRect(chars);
  const QRect textWidget = text.widget().adjusted(0, 0, -glob_margin, 0);
  return {
    {textWidget, text.outer(), text.inner(), text.pos()},
    {iconWidget, iconOuter, iconInner, iconPos}
  };
}

struct IntRange {
  int min;
  int max;
  int def = 0;
};

// -------------------------------- tools ----------------------------------- //

constexpr QSize     tool_icon_size = {24_px, 24_px};
constexpr QSize     tool_icon_padding = {1_px, 1_px};
constexpr QSize     tool_button_size = tool_icon_size + 2 * tool_icon_padding;
constexpr QPoint    tool_icon_pos = toPoint(tool_icon_padding);
constexpr int       tool_select_width = tool_button_size.width() + 2 * glob_margin;

inline const QColor tool_select_background = glob_main;
inline const QColor tool_base_disabled = glob_light_1;
inline const QColor tool_base_enabled = glob_light_2;
inline const QColor tool_shape_disabled = glob_light_2;
inline const QColor tool_shape_enabled = glob_light_1;

constexpr IntRange  tool_overlay_gray = {32, 223, 127};
constexpr IntRange  tool_overlay_alpha = {32, 191, 127};
constexpr QRgb      tool_overlay_color = qRgba(
  tool_overlay_gray.def,
  tool_overlay_gray.def,
  tool_overlay_gray.def,
  tool_overlay_alpha.def
);

constexpr int scaleOverlay(const IntRange range, const int value) {
  return value * (range.max - range.min) / 255 + range.min;
}
constexpr int scaleOverlayGray(const int gray) {
  return scaleOverlay(tool_overlay_gray, gray);
}
constexpr int scaleOverlayAlpha(const int alpha) {
  return scaleOverlay(tool_overlay_alpha, alpha);
}

// ------------------------------- status bar ------------------------------- //

constexpr int       stat_temp_duration_ms = 2500;
constexpr int       stat_min_width = 200_px;
constexpr int       stat_height = glob_font_px + 2 * (glob_text_margin + glob_margin);

inline const QColor stat_background = glob_main;

// --------------------------------- editor --------------------------------- //

constexpr int       edit_min_scale = 1;
constexpr int       edit_max_scale = 64;
constexpr int       edit_undo_stack = 128;

inline const QRgb   edit_checker_a = qRgb(191, 191, 191);
inline const QRgb   edit_checker_b = qRgb(255, 255, 255);

// ------------------------------ color picker ------------------------------ //

constexpr WidgetRect pick_svgraph_rect = boxRect(101_px, 101_px);
constexpr WidgetRect pick_slider_rect = boxRect(pick_svgraph_rect.inner().width(), 12_px);
constexpr WidgetRect pick_label_rect = textBoxRect(1, 1_px);
constexpr WidgetRect pick_number_rect = textBoxRect(3);
constexpr WidgetRect pick_hex_rect = basicRect(
  QPoint{glob_text_margin + 2_px, glob_text_margin},
  textBoxSize(8, 2_px + 3_px)
);
constexpr WidgetRect pick_name_rect = basicRect(
  toPoint(glob_text_margin),
  QSize{pick_svgraph_rect.inner().width(), glob_font_px + 2 * glob_text_margin}
);

inline const QColor pick_primary_color = {0, 0, 0};
inline const QColor pick_secondary_color = {255, 255, 255};
inline const QColor pick_default_color = {255, 0, 0, 255};
constexpr int       pick_default_gray = 255;

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

constexpr WidgetRect tool_color_rect = basicRect({}, {24_px, 12_px}, 1_px, 2_px);
constexpr WidgetRect active_color_rect = basicRect({}, {24_px, 12_px}, 2_px, 1_px);
constexpr int        tool_color_tiles = 2;
constexpr WidgetRect tool_colors_rect = basicRect(
  {}, {pick_svgraph_rect.inner().width(), 12_px}, 0_px, 3_px
);

// ------------------------------ palette colors ---------------------------- //

constexpr int       pal_colors = 256;
constexpr int       pal_width = 8;
constexpr int       pal_height = pal_colors / pal_width;
constexpr int       pal_tile_size = 13_px;

// --------------------------------- timeline ------------------------------- //

inline const QColor cell_icon_color = glob_light_2;
inline const QColor cell_curr_color = glob_light_1;
inline const QColor cell_select_color = glob_dark_1;

constexpr int       cell_icon_pad = 1_px;
constexpr int       cell_icon_size = 7_px;
constexpr int       cell_width = 2 * cell_icon_pad + cell_icon_size + glob_border_width;
constexpr int       cell_height = cell_width;
constexpr int       cell_border_offset = cell_icon_pad + glob_border_width;
constexpr int       frame_incr = 5;
constexpr int       layer_name_max_len = 256;

constexpr WidgetRect timelineTextBox(const int innerWidth) {
  const QPoint pos = {
    glob_text_margin,
    cell_height - glob_border_width - glob_text_margin - glob_font_px
  };
  const QSize innerSize = {
    innerWidth,
    cell_height - glob_border_width
  };
  const QSize outerSize = innerSize + toSize(glob_border_width);
  return {toRect(outerSize), toRect(outerSize), toRect(innerSize), pos};
}

constexpr IntRange   ctrl_delay = {1, 999, 100};
constexpr WidgetRect ctrl_text_rect = timelineTextBox(textBoxWidth(3));
constexpr int        layer_width = 7 * cell_width + ctrl_text_rect.widget().width();
constexpr WidgetRect layer_text_rect = timelineTextBox(layer_width - cell_width - glob_border_width);

// ---------------------------- init canvas dialog -------------------------- //

constexpr IntRange   init_size_range = {1, 65536, 128};
constexpr WidgetRect init_size_rect = textBoxRect(5);
constexpr WidgetRect init_button_rect = textBoxRect(8);

// ------------------------------- export dialog ---------------------------- //

constexpr IntRange   expt_stride = {-100, 100, 1};
constexpr IntRange   expt_offset = {-100, 100, 0};
constexpr IntRange   expt_scale = {-32, 32, 1};

// -------------------------------- error dialog ---------------------------- //

constexpr int        erro_wrap_width = 40;

// -------------------------------- file format ----------------------------- //

constexpr size_t file_sig_len = 8;
constexpr char file_sig[file_sig_len] = "animera";

#define NAME(STR)                                                               \
  {STR[0], STR[1], STR[2], STR[3]};                                             \
  static_assert(sizeof(STR) == sizeof(char) * 5)

constexpr size_t chunk_name_len = 4;
constexpr char chunk_palette[chunk_name_len] = NAME("PLTE");
constexpr char chunk_anim_header[chunk_name_len] = NAME("AHDR");
constexpr char chunk_layer_header[chunk_name_len] = NAME("LHDR");
constexpr char chunk_cell_header[chunk_name_len] = NAME("CHDR");
constexpr char chunk_cell_data[chunk_name_len] = NAME("CDAT");
constexpr char chunk_anim_end[chunk_name_len] = NAME("AEND");

#undef NAME

constexpr size_t file_buff_size = 1 << 15;
constexpr size_t file_int_size = 4;

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
constexpr Qt::Key   key_sample = Qt::Key_B;
constexpr Qt::Key   key_undo = Qt::Key_N;
constexpr Qt::Key   key_redo = Qt::Key_M;

// brush
constexpr IntRange  brsh_radius = {0, 64, 0};

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
