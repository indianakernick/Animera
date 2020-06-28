//
//  config keys.hpp
//  Animera
//
//  Created by Indiana Kernick on 15/6/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_config_keys_hpp
#define animera_config_keys_hpp

#include <QtGui/qkeysequence.h>

// all tools
constexpr Qt::MouseButton mouse_primary = Qt::LeftButton;
constexpr Qt::MouseButton mouse_secondary = Qt::RightButton;
constexpr Qt::MouseButton mouse_tertiary = Qt::MiddleButton;

constexpr Qt::Key   key_primary = Qt::Key_V;
constexpr Qt::Key   key_secondary = Qt::Key_C;
constexpr Qt::Key   key_tertiary = Qt::Key_X;
constexpr Qt::Key   key_tool_up = Qt::Key_Q;
constexpr Qt::Key   key_tool_down = Qt::Key_A;

// editor
constexpr Qt::Key   key_zoom_out = Qt::Key_W;
constexpr Qt::Key   key_zoom_in = Qt::Key_R;
constexpr Qt::Key   key_zoom_fit = Qt::Key_T;
constexpr Qt::Key   key_sample = Qt::Key_Z;
constexpr Qt::Key   key_undo = Qt::Key_G;
constexpr Qt::Key   key_redo = Qt::Key_B;

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

// file
constexpr auto       key_new_file = QKeySequence::New;
constexpr auto       key_open_file = QKeySequence::Open;
constexpr auto       key_save_file = QKeySequence::Save;
constexpr auto       key_save_file_as = QKeySequence::SaveAs;
inline const QString key_export_file = "CTRL+E";
inline const QString key_export_frame = "CTRL+SHIFT+E";
inline const QString key_export_cel = "CTRL+ALT+E";
inline const QString key_import_cel = "CTRL+ALT+I";

// layer
inline const QString key_new_layer = "SHIFT+N";
inline const QString key_delete_layer = "SHIFT+BACKSPACE";
inline const QString key_move_layer_up = "SHIFT+E";
inline const QString key_move_layer_down = "SHIFT+D";
inline const QString key_toggle_layer_vis = "SHIFT+V";
inline const QString key_isolate_layer = "SHIFT+B";
constexpr Qt::Key    key_layer_above = Qt::Key_E;
constexpr Qt::Key    key_layer_below = Qt::Key_D;

// frame
inline const QString key_new_frame = "ALT+N";
inline const QString key_delete_frame = "ALT+BACKSPACE";
inline const QString key_clear_cel = "ALT+C";
inline const QString key_extend_cel = "ALT+E";
inline const QString key_split_cel = "ALT+S";
constexpr Qt::Key    key_next_frame = Qt::Key_F;
constexpr Qt::Key    key_prev_frame = Qt::Key_S;
constexpr Qt::Key    key_play_anim = Qt::Key_Space;

// selection
inline const QString key_clear_selection = "CTRL+X";
constexpr auto       key_copy_selection = QKeySequence::Copy;
constexpr auto       key_paste_selection = QKeySequence::Paste;

// palette
inline const QString key_reset_palette = {};
inline const QString key_open_palette = {};
inline const QString key_save_palette = {};

#endif
