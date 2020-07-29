//
//  config geometry.hpp
//  Animera
//
//  Created by Indiana Kernick on 26/3/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_config_geometry_hpp
#define animera_config_geometry_hpp

#include "geometry.hpp"
#include "int range.hpp"
#include "widget rect.hpp"

// --------------------------------- global --------------------------------- //

constexpr int       glob_scale = 2;

constexpr int operator""_px(const unsigned long long literal) {
  return static_cast<int>(literal * glob_scale);
}

constexpr int       glob_font_width_px = 5_px;
constexpr int       glob_font_kern_px = 1_px;
constexpr int       glob_font_stride_px = glob_font_width_px + glob_font_kern_px;
constexpr int       glob_font_ascent_px = 7_px;
constexpr int       glob_font_descent_px = 0_px;
constexpr int       glob_font_px = glob_font_ascent_px + glob_font_descent_px;

#ifdef Q_OS_WIN
constexpr int       glob_font_pt = 6_px; // 6 pt for 96 dpi
#else
constexpr int       glob_font_pt = 8_px; // 8 pt for 72 dpi
#endif

constexpr QSize     glob_window_size = {640_px, 360_px};

// ---------------------------- global dimensions --------------------------- //

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

constexpr QSize textBoxSize(const QSize chars) {
  return {textBoxWidth(chars.width()), textBoxHeight(chars.height())};
}

constexpr WidgetRect textBoxRect(const int chars, const int offsetX = 0) {
  return basicRect(
    {glob_text_margin + offsetX, glob_text_margin},
    textBoxSize(chars, offsetX)
  );
}

constexpr WidgetRect textBoxRect(const QSize chars) {
  return basicRect(
    toPoint(glob_text_margin),
    textBoxSize(chars)
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

constexpr WidgetRect addMargins(
  const WidgetRect &rect,
  const int left,
  const int top,
  const int right,
  const int bottom
) {
  const QSize widgetSize = rect.widget().size() + QSize{left + right, top + bottom};
  const QPoint outerPos = rect.outer().topLeft() + QPoint{left, top};
  const QPoint innerPos = rect.inner().topLeft() + QPoint{left, top};
  const QPoint pos = rect.pos() + QPoint{left, top};
  return {
    {rect.widget().topLeft(), widgetSize},
    {outerPos, rect.outer().size()},
    {innerPos, rect.inner().size()},
    pos
  };
}

constexpr WidgetRect addMargins(
  const WidgetRect &rect,
  const bool left,
  const bool top,
  const bool right,
  const bool bottom
) {
  return addMargins(
    rect,
    left * glob_margin,
    top * glob_margin,
    right * glob_margin,
    bottom * glob_margin
  );
}

constexpr WidgetRect addMargins(const WidgetRect &rect) {
  return addMargins(rect, true, true, true, true);
}

// ------------------------------- tool select ------------------------------ //

constexpr QSize     tool_icon_size = {24_px, 24_px};
constexpr QSize     tool_icon_padding = {1_px, 1_px};
constexpr QSize     tool_button_size = tool_icon_size + 2 * tool_icon_padding;
constexpr QPoint    tool_icon_pos = toPoint(tool_icon_padding);
constexpr int       tool_select_width = tool_button_size.width() + 2 * glob_margin;

// ------------------------------- status bar ------------------------------- //

constexpr int        stat_temp_duration_ms = 2500;
constexpr WidgetRect stat_rect = addMargins(textBoxRect(48));

// --------------------------------- editor --------------------------------- //

constexpr int       edit_min_scale = 1;
constexpr int       edit_max_scale = 64;
constexpr int       edit_undo_stack = 32;

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

constexpr int       pick_alpha_tiles = 2;

// -------------------------------- text box -------------------------------- //

constexpr int       box_cursor_blink_interval_ms = 500;
constexpr int       box_cursor_width = glob_font_kern_px;

// ------------------------------- tool colors ------------------------------ //

constexpr WidgetRect tool_color_rect = basicRect({}, {24_px, 12_px}, 1_px, 2_px);
constexpr WidgetRect active_color_rect = basicRect({}, {24_px, 12_px}, 2_px, 1_px);
constexpr int        tool_color_tiles = 2;
constexpr WidgetRect tool_colors_rect = basicRect(
  {}, {pick_svgraph_rect.inner().width(), 12_px}, 0_px, 3_px
);

// ----------------------------- palette colors ----------------------------- //

constexpr int       pal_colors = 256;
constexpr int       pal_width = 8;
constexpr int       pal_height = pal_colors / pal_width;
constexpr int       pal_tile_size = 12_px;
constexpr int       pal_tile_stride = pal_tile_size + glob_border_width;

// -------------------------------- timeline -------------------------------- //

constexpr int       cel_icon_pad = 1_px;
constexpr int       cel_icon_size = 7_px;
constexpr int       cel_width = 2 * cel_icon_pad + cel_icon_size + glob_border_width;
constexpr int       cel_height = cel_width;
constexpr int       cel_border_offset = cel_icon_pad + glob_border_width;
constexpr int       frame_incr = 5;
constexpr int       layer_name_max_len = 256;
constexpr int       group_boundary_radius = 1_px;

constexpr WidgetRect timelineTextBox(const int innerWidth) {
  const QPoint pos = {
    glob_text_margin,
    cel_height - glob_border_width - glob_text_margin - glob_font_px
  };
  const QSize innerSize = {
    innerWidth,
    cel_height - glob_border_width
  };
  const QSize outerSize = innerSize + toSize(glob_border_width);
  return {toRect(outerSize), toRect(outerSize), toRect(innerSize), pos};
}

constexpr IntRange   ctrl_delay = {1, 999, 100};
constexpr WidgetRect ctrl_text_rect = timelineTextBox(textBoxWidth(3));
constexpr int        layer_width = 7 * cel_width + ctrl_text_rect.widget().width();
constexpr WidgetRect layer_text_rect = timelineTextBox(layer_width - cel_width - glob_border_width);

// --------------------------- init canvas dialog --------------------------- //

constexpr IntRange   init_size_range = {1, 32768, 128};
constexpr WidgetRect init_size_rect = textBoxRect(5);
constexpr WidgetRect init_button_rect = textBoxRect(8);

// ------------------------------ export dialog ----------------------------- //

constexpr IntRange   expt_scale = {-64, 64, 1};

// ------------------------------ error dialog ------------------------------ //

constexpr int        erro_wrap_width = 40;

// ------------------------------- keys dialog ------------------------------ //

constexpr int        keys_width = 20;

// ------------------------------- file format ------------------------------ //

constexpr std::size_t file_sig_len = 8;
constexpr char file_sig[file_sig_len] = "animera";

constexpr std::size_t chunk_name_len = 4;
constexpr char chunk_anim_header[chunk_name_len + 1] = "AHDR";
constexpr char chunk_palette[chunk_name_len + 1] = "PLTE";
constexpr char chunk_groups[chunk_name_len + 1] = "GRPS";
constexpr char chunk_layer_header[chunk_name_len + 1] = "LHDR";
constexpr char chunk_cel_header[chunk_name_len + 1] = "CHDR";
constexpr char chunk_cel_data[chunk_name_len + 1] = "CDAT";
constexpr char chunk_anim_end[chunk_name_len + 1] = "AEND";

constexpr std::size_t file_buff_size = 1 << 15;
constexpr std::size_t file_int_size = 4;

static_assert(sizeof(int) >= sizeof(std::uint32_t));

// TODO: enforce max image size
constexpr int max_image_width = ~std::uint32_t{} / 4;
constexpr int max_image_height = max_image_width;

// ---------------------------------- tools --------------------------------- //

// brush
constexpr IntRange  brsh_radius = {0, 64, 0};

// line
constexpr IntRange  line_radius = brsh_radius;

// stroked circle
constexpr IntRange  circ_thick = {1, 64, 1};

// stroked rectangle
constexpr IntRange  rect_thick = circ_thick;

// wand
constexpr int       wand_alpha = 128;
constexpr int       wand_frames = 8;
constexpr int       wand_interval = 100;

#endif
