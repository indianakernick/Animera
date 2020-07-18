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
  /// One-past-the-end frame for this group or the beginning of the next group.
  FrameIdx end;
  /// Name of the group.
  std::string name;
};

/// @brief Group iterator that iterates frames.
///
/// Incrementing this iterator increments the frame. Information about the group
/// under that frame is returned. This isn't a proper iterator (with all the
/// types and operators) but it acts like one.
class GroupIterator {
public:
  /// @brief Construct an iterator to the group under the given frame.
  ///
  /// @note Asserts if the frame is out of range.
  /// @sa findGroup
  ///
  /// @param array Group array.
  /// @param frame Index of the frame.
  GroupIterator(tcb::span<const Group> array, FrameIdx frame);
  
  /// @brief Increment the iterator to the next frame.
  ///
  /// @note Asserts if incrementing past the end.
  ///
  /// @returns Whether the iterator advanced to a new group.
  bool incr();
  
  /// @brief Get the group at the iterator.
  ///
  /// @note Asserts if the iterator is at the end.
  /// @sa getGroup
  ///
  /// @returns The index of the group and its boundaries.
  GroupInfo info() const;
  
  /// @brief Get the name of the group at the iterator.
  ///
  /// @note Asserts if the iterator is at the end.
  ///
  /// @returns The name of the group.
  std::string_view name() const;
  
private:
  tcb::span<const Group> array;
  tcb::span<const Group>::iterator iter;
  FrameIdx frame;
};

/// @brief Move the group boundary to the given frame.
///
/// The frame is clamped so that moving the boundary doesn't resize any group to
/// zero length.
///
/// @note Asserts if the group index is out of range.
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
/// @note Asserts if the frame index is out of range.
///
/// @param array Group array.
/// @param frame Index of the frame to insert.
void insertGroupFrame(std::vector<Group> &array, FrameIdx frame);

/// @brief Remove a frame from a group.
///
/// The group under the frame is resized and all groups to the right are shifted
/// left. If the group is resized to a zero length, it will be removed entirely.
///
/// @note Asserts if the frame index is out of range.
///
/// @param array Group array.
/// @param frame Index of frame to remove.
/// @returns Index of the group that was removed or null if the group was only
/// resized.
std::optional<GroupIdx> removeGroupFrame(std::vector<Group> &array, FrameIdx frame);

/// @brief Split a group at the given boundary, creating a new group on the
/// left.
///
/// The given frame becomes the "end" frame for a new group created on the left
/// side of the group under the frame. The group cannot be resized to zero size.
///
/// @note Asserts if the frame index is out of range.
/// @sa splitGroupRight
///
/// @param array Group array.
/// @param frame Desired "end" frame for the left group.
/// @returns Whether the group was actually split.
bool splitGroupLeft(std::vector<Group> &array, FrameIdx frame);

/// @brief Split a group at the given boundary, creating a new group on the
/// right.
///
/// The given frame becomes the "end" frame for the group under the frame. A new
/// group is created on the right side of the existing one to fill the space.
/// The group cannot be resized to zero size.
///
/// @note Asserts if the frame index is out of range.
/// @sa splitGroupLeft
///
/// @param array Group array.
/// @param frame Desired "end" frame for the left group.
/// @returns Whether the group was actually split.
bool splitGroupRight(std::vector<Group> &array, FrameIdx frame);

/// @brief Merge the given group with the group on its left.
///
/// The group on the left is removed if it exists.
///
/// @note Asserts if the group index is out of range.
/// @sa mergeGroupRight
///
/// @param array Group array.
/// @param group Index of the group to merge.
/// @returns Whether the group was actually merged.
bool mergeGroupLeft(std::vector<Group> &array, GroupIdx group);

/// @brief Merge the given group with the group on its right.
///
/// The group on the right is removed if it exists and the given group is
/// resized to fill the space.
///
/// @note Asserts if the group index is out of range.
/// @sa mergeGroupLeft
///
/// @param array Group array.
/// @param group Index of the group to merge.
/// @returns Whether the group was actually merged.
bool mergeGroupRight(std::vector<Group> &array, GroupIdx group);

/// @brief Find a group given its boundary.
///
/// @param array Group array.
/// @param frame The "end" frame of the desired group.
/// @returns Index of the group on the left side of the boundary or null if the
/// frame isn't on a boundary.
std::optional<GroupIdx> findGroupBoundary(tcb::span<const Group> array, FrameIdx frame);

/// @brief Find the group under the given frame.
///
/// @note Asserts if the frame is out of range.
///
/// @param array Group array.
/// @param frame Index of the frame.
/// @returns The index of the group and its boundaries.
GroupInfo findGroup(tcb::span<const Group> array, FrameIdx frame);

/// @brief Get the given group.
///
/// @note Asserts if the group index is out of range.
///
/// @param array Group array.
/// @param group Index of the group to get.
/// @returns The index of the group and its boundaries.
GroupInfo getGroup(tcb::span<const Group> array, GroupIdx group);

#endif
