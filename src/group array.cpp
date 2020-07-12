//
//  group array.cpp
//  Animera
//
//  Created by Indiana Kernick on 12/7/20.
//  Copyright © 2020 Indiana Kernick. All rights reserved.
//

#include "group array.hpp"

namespace {

template <typename Iterator>
auto findGroupIter(const Iterator begin, const Iterator end, FrameIdx &idx) {
  if (idx < FrameIdx{0}) {
    return end;
  }
  const auto compare = [](const FrameIdx idx, const Group &group) {
    return idx < group.end;
  };
  const auto iter = std::upper_bound(begin, end, idx, compare);
  if (iter != begin) {
    idx -= std::prev(iter)->end;
  }
  return iter;
}

}

bool moveGroupBoundary(
  std::vector<Group> &groups,
  const GroupIdx group,
  FrameIdx end
) {
  assert(groups.size() >= 2);
  assert(GroupIdx{0} <= group);
  assert(static_cast<std::size_t>(group) < groups.size() - 1);
  
  const FrameIdx min = group == GroupIdx{0} ? FrameIdx{1} : groups[+group - 1].end + FrameIdx{1};
  const FrameIdx max = groups[+group + 1].end - FrameIdx{1};
  end = std::clamp(end, min, max);
  if (groups[+group].end != end) {
    groups[+group].end = end;
    return true;
  } else {
    return false;
  }
}

void insertGroupFrame(std::vector<Group> &groups, FrameIdx frame) {
  auto iter = findGroupIter(groups.begin(), groups.end(), frame);
  assert(iter != groups.end());
  
  for (; iter != groups.end(); ++iter) {
    ++iter->end;
  }
}

std::optional<GroupIdx> removeGroupFrame(
  std::vector<Group> &groups,
  FrameIdx frame
) {
  auto iter = findGroupIter(groups.begin(), groups.end(), frame);
  assert(iter != groups.end());
  
  std::optional<GroupIdx> group;
  const FrameIdx prevEnd = iter == groups.begin() ? FrameIdx{0} : std::prev(iter)->end;
  
  --iter->end;
  if (iter->end == prevEnd) {
    group = static_cast<GroupIdx>(iter - groups.begin());
    iter = groups.erase(iter);
  } else {
    ++iter;
  }
  
  for (; iter != groups.end(); ++iter) {
    --iter->end;
  }
  
  return group;
}

bool splitGroupLeft(std::vector<Group> &groups, const FrameIdx frame) {
  FrameIdx offset = frame;
  auto iter = findGroupIter(groups.begin(), groups.end(), offset);
  assert(iter != groups.end());
  if (offset == FrameIdx{0}) return false;
  
  Group group{frame, "Group " + std::to_string(groups.size())};
  groups.insert(iter, std::move(group));
  return true;
}

bool splitGroupRight(std::vector<Group> &groups, const FrameIdx frame) {
  FrameIdx offset = frame;
  auto iter = findGroupIter(groups.begin(), groups.end(), offset);
  assert(iter != groups.end());
  if (offset == FrameIdx{0}) return false;
  
  Group group{iter->end, "Group " + std::to_string(groups.size())};
  iter->end = frame;
  groups.insert(++iter, std::move(group));
  return true;
}

bool mergeGroupLeft(std::vector<Group> &groups, const GroupIdx group) {
  assert(group >= GroupIdx{0});
  assert(static_cast<std::size_t>(group) < groups.size());
  if (group == GroupIdx{0}) return false;
  
  groups.erase(groups.begin() + (+group - 1));
  return true;
}

bool mergeGroupRight(std::vector<Group> &groups, const GroupIdx group) {
  assert(group >= GroupIdx{0});
  assert(static_cast<std::size_t>(group) < groups.size());
  if (static_cast<std::size_t>(group) == groups.size() - 1) return false;
  
  const auto next = groups.begin() + (+group + 1);
  groups[+group].end = next->end;
  groups.erase(next);
  return true;
}

std::optional<GroupIdx> findGroupBoundary(
  const tcb::span<const Group> groups,
  const FrameIdx end
) {
  if (groups.empty()) return std::nullopt;
  if (end <= FrameIdx{0} || end >= groups.back().end) return std::nullopt;
  
  const auto compare = [](const Group &group, const FrameIdx frame) {
    return group.end < frame;
  };
  const auto iter = std::lower_bound(groups.begin(), groups.end(), end, compare);
  if (iter == groups.end()) return std::nullopt;
  if (iter->end != end) return std::nullopt;
  
  return static_cast<GroupIdx>(iter - groups.begin());
}

GroupInfo findGroup(const tcb::span<const Group> groups, FrameIdx frame) {
  const auto iter = findGroupIter(groups.begin(), groups.end(), frame);
  assert(iter != groups.end());
  
  GroupInfo info;
  info.group = static_cast<GroupIdx>(iter - groups.begin());
  info.begin = iter == groups.begin() ? FrameIdx{0} : std::prev(iter)->end;
  info.end = iter->end;
  return info;
}

GroupInfo getGroup(const tcb::span<const Group> groups, const GroupIdx group) {
  GroupInfo info;
  info.group = group;
  info.begin = group == GroupIdx{0} ? FrameIdx{0} : groups[+group - 1].end;
  info.end = groups[+group].end;
  return info;
}
