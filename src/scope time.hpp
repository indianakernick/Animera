//
//  scope time.hpp
//  Animera
//
//  Created by Indi Kernick on 1/12/19.
//  Copyright © 2019 Indi Kernick. All rights reserved.
//

#ifndef scope_time_hpp
#define scope_time_hpp

#ifdef ENABLE_SCOPE_TIME

#include <chrono>
#include <cstdio>
#include <vector>
#include <unordered_map>

class ScopeTime {
public:
  explicit ScopeTime(const char *);
  ~ScopeTime();
  
  static void print();
  static void reset();
  
private:
  using Clock = std::chrono::high_resolution_clock;

  Clock::time_point start;

  struct TreeNode {
    size_t calls;
    Clock::duration time;
    std::unordered_map<const char *, TreeNode> children;
    const char *name;
    TreeNode *parent;
  };
  
  static inline TreeNode tree {0, {}, {}, "ROOT", nullptr};
  static inline TreeNode *current = &tree;
  
  static void printImpl(const TreeNode *, int);
  
  static constexpr int name_indent = 2;
  static constexpr int max_depth = 32;
  static constexpr int num_prec = 4;
  static constexpr int name_width = 48;
  static constexpr int rest_width = 24;
  
  static inline char spaces[name_indent * max_depth] {};
  static inline bool initSpaces = false;
};

inline ScopeTime::ScopeTime(const char *name) {
  TreeNode *prevCurrent = current;
  current = &current->children[name];
  current->parent = prevCurrent;
  current->name = name;
  start = Clock::now();
}

inline ScopeTime::~ScopeTime() {
  current->time += Clock::now() - start;
  ++current->calls;
  current = current->parent;
}

inline void ScopeTime::print() {
  std::printf("%-*s", name_width, "Name");
  std::printf("%-*s", rest_width, "Total Count");
  std::printf("%-*s", rest_width, "Avg Count per parent");
  std::printf("%-*s", rest_width, "Total Time (ms)");
  std::printf("%-*s", rest_width, "Average Time (ms)");
  std::printf("%-*s", rest_width, "Percent of parent time");
  std::printf("\n");

  printImpl(&tree, 0);
}

inline void ScopeTime::reset() {
  current = &tree;
  tree.calls = 0;
  tree.time = {};
  tree.children.clear();
  tree.name = "ROOT";
  tree.parent = nullptr;
}

inline void ScopeTime::printImpl(const TreeNode *node, const int depth) {
  assert(depth <= max_depth);
  
  int newDepth = 0;
  if (node->parent) {
    newDepth = depth + 1;
    
    if (!std::exchange(initSpaces, true)) {
      std::memset(spaces, ' ', sizeof(spaces));
    }
    
    std::printf("%.*s", depth * name_indent, spaces);
    std::printf("%-*s", name_width - depth * name_indent, node->name);
    
    std::printf("%-*zu", rest_width, node->calls);
    
    //not child of root
    if (node->parent->parent) {
      const double avgParentCalls = static_cast<double>(node->calls) / node->parent->calls;
      std::printf("%-*.*g", rest_width, num_prec, avgParentCalls);
    } else {
      std::printf("%-*zu", rest_width, node->calls);
    }

    using MilliFloat = std::chrono::duration<double, std::milli>;
    const double totalTime = std::chrono::duration_cast<MilliFloat>(node->time).count();
    std::printf("%-*.*g", rest_width, num_prec, totalTime);

    if (node->calls) {
      std::printf("%-*.*g", rest_width, num_prec, totalTime / node->calls);
    } else {
      std::printf("%-*i", rest_width, 0);
    }
    
    //not child of root
    if (node->parent->parent) {
      const double percent = (static_cast<double>(node->time.count()) / node->parent->time.count());
      std::printf("%.*g%%", num_prec, percent * 100.0);
    }
    
    std::printf("\n");
  }
  
  //copy children into a vector
  std::vector<const TreeNode *> children;
  children.reserve(node->children.size());
  for (auto i = node->children.begin(); i != node->children.end(); ++i) {
    children.push_back(&(i->second));
  }
  
  //sort by total time in accending order
  std::sort(children.begin(), children.end(), [](const TreeNode *a, const TreeNode *b) {
    return a->time > b->time;
  });
  
  for (const TreeNode *child : children) {
    printImpl(child, newDepth);
  }
}

#define CONCAT_IMPL(A, B) A##B
#define CONCAT(A, B) CONCAT_IMPL(A, B)
#define SCOPE_TIME(NAME) ScopeTime CONCAT(scope_time_, __LINE__) {NAME}
#define SCOPE_TIME_PRINT() ScopeTime::print()
#define SCOPE_TIME_RESET() ScopeTime::reset()

#else

#define SCOPE_TIME(NAME)
#define SCOPE_TIME_PRINT()
#define SCOPE_TIME_RESET()

#endif

#endif
