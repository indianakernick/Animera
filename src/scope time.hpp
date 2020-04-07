//
//  scope time.hpp
//  Animera
//
//  Created by Indiana Kernick on 1/12/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#ifndef animera_scope_time_hpp
#define animera_scope_time_hpp

#ifdef ENABLE_SCOPE_TIME

#include <chrono>
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
    std::size_t calls = 0;
    Clock::duration time;
    std::unordered_map<const char *, TreeNode> children;
    const char *name;
    TreeNode *parent;
  };
  
  static inline TreeNode tree {0, {}, {}, "ROOT", nullptr};
  static inline TreeNode *current = &tree;
  
  static void printImpl(const TreeNode *, int);
  
  static constexpr int name_indent = 2;
  static constexpr int num_prec = 4;
  static constexpr int name_width = 48;
  static constexpr int rest_width = 24;
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

#define SCOPE_TIME_IMPL2(NAME, LINE) ScopeTime scope_time_##LINE {NAME}
#define SCOPE_TIME_IMPL(NAME, LINE) SCOPE_TIME_IMPL2(NAME, LINE)
#define SCOPE_TIME(NAME) SCOPE_TIME_IMPL(NAME, __LINE__)
#define NO_SCOPE_TIME(NAME) SCOPE_TIME(nullptr)
#define SCOPE_TIME_PRINT() ScopeTime::print()
#define SCOPE_TIME_RESET() ScopeTime::reset()

#else

#define SCOPE_TIME(NAME)
#define NO_SCOPE_TIME(NAME)
#define SCOPE_TIME_PRINT()
#define SCOPE_TIME_RESET()

#endif

#endif
