//
//  group array.hpp
//  Animera
//
//  Created by Indiana Kernick on 12/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#ifndef animera_group_array_hpp
#define animera_group_array_hpp

#include <vector>
#include "cel.hpp"
#include <optional>
#include <tcb/span.hpp>

struct Group {
  FrameIdx end;
  std::string name;
};

/// @brief Move the group boundary to the given frame.
///
/// The frame is clamped so that moving the boundary doesn't resize any group to
/// zero length.
///
/// @param array Group array.
/// @param group Index of the group to resize.
/// @param frame Desired "end" frame for the group.
/// @returns Whether the boundary was actually moved.
bool moveGroupBoundary(std::vector<Group> &array, GroupIdx group, FrameIdx frame);

/// @brief Insert a frame into a group.
///
/// The group under the frame is resized and all groups to the right are shifted
/// right.
///
/// @param array Group array.
/// @param frame Index of the frame to insert.
void insertGroupFrame(std::vector<Group> &array, FrameIdx frame);

/// @brief Remove a frame from a group.
///
/// The group under the frame is resized and all groups to the right are shifted
/// left. If the group is resized to a zero length, it will be removed entirely.
///
/// @param array Group array.
/// @param frame Index of frame to remove.
/// @returns Index of the group that was removed or null if the group was only
/// resized.
std::optional<GroupIdx> removeGroupFrame(std::vector<Group> &array, FrameIdx frame);

/// @brief Split a group at the given boundary.
///
/// The given frame becomes the "end" frame for the group under the frame. A new
/// group is created on the right side of the existing one to fill the space.
/// The group cannot be resized to zero size.
///
/// @param array Group array.
/// @param frame Desired "end" frame for the left group.
/// @returns Whether the group was actually split.
bool splitGroup(std::vector<Group> &array, FrameIdx frame);

/// @brief Find a group given its boundary.
///
/// @param array Group array.
/// @param frame The "end" frame of the desired group.
/// @returns Index of the group on the left side of the boundary or null if the
/// frame isn't on a boundary.
std::optional<GroupIdx> findGroupBoundary(tcb::span<const Group> array, FrameIdx frame);

/// @brief Find the group under the given frame.
///
/// @param array Group array.
/// @param frame Index of the frame.
/// @returns The index of the group and its boundaries.
GroupInfo findGroup(tcb::span<const Group> array, FrameIdx frame);

/// @brief Get the given group.
///
/// @param array Group array.
/// @param group Index of the group to get.
/// @returns The index of the group and its boundaries.
GroupInfo getGroup(tcb::span<const Group> array, GroupIdx group);

#endif
