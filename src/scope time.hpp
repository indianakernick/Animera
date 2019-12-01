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

// TODO: refactor Profiler
// I first wrote this in 2016
// It hasn't really changed much since

#include <chrono>
#include <cstdio>
#include <vector>
#include <unordered_map>

class Profiler {
public:
  explicit Profiler(const char *name);
  ~Profiler();
  
  static void formatInfo(std::FILE *);
  static void resetInfo();
  
private:
  struct TreeNode {
    uint64_t calls;
    uint64_t time;
    std::unordered_map<const char *, TreeNode> children;
    const char *name;
    TreeNode *parent;
  };
  
  std::chrono::high_resolution_clock::time_point start;
  static inline TreeNode tree {0, 0, {}, "ROOT", nullptr};
  static inline TreeNode *current = &tree;
  
  static void recFormatInfo(std::FILE *, TreeNode *, int);
  
  static const int NAME_INDENT = 2;
  static const int MAX_DEPTH = 16;
  static const int NUM_PREC = 4;
  static const int NAME_WIDTH = 48;
  static const int REST_WIDTH = 24;
  
  static inline char spaces[NAME_INDENT * MAX_DEPTH] {};
  static inline bool initSpaces = false;
  static inline bool oddLine = false;
};

inline Profiler::Profiler(const char *name) {
  TreeNode *prevCurrent = current;
  current = &current->children[name];
  current->parent = prevCurrent;
  current->name = name;
  start = std::chrono::high_resolution_clock::now();
}

inline Profiler::~Profiler() {
  current->time += (std::chrono::high_resolution_clock::now() - start).count();
  ++current->calls;
  current = current->parent;
}

inline void Profiler::formatInfo(std::FILE *stream) {
  std::fprintf(stream, "%-*s", NAME_WIDTH, "Name");
  std::fprintf(stream, "%-*s", REST_WIDTH, "Total Count");
  std::fprintf(stream, "%-*s", REST_WIDTH, "Avg Count per parent");
  std::fprintf(stream, "%-*s", REST_WIDTH, "Total Time (ms)");
  std::fprintf(stream, "%-*s", REST_WIDTH, "Average Time (ms)");
  std::fprintf(stream, "%-*s", REST_WIDTH, "Percent of parent time");
  std::fprintf(stream, "\n");

  recFormatInfo(stream, &tree, 0);
  /*
  The Xcode console doesn't support ANSI escape codes
  if (stream == stdout || stream == stderr) {
    Term::defaultBackColor();
  }*/
}

inline void Profiler::resetInfo() {
  current = &tree;
  tree.calls = 0;
  tree.time = 0;
  tree.children.clear();
  tree.name = "ROOT";
  tree.parent = nullptr;
}

inline void Profiler::recFormatInfo(std::FILE *stream, TreeNode *node, int depth) {
  int newDepth;
  if (node->parent) {
    newDepth = depth + 1;
    
    if (!initSpaces) {
      std::memset(spaces, ' ', sizeof(spaces));
      initSpaces = true;
    }
    /*
    The Xcode console doesn't support ANSI escape codes
    if (stream == stdout || stream == stderr) {
      if (oddLine) {
        Term::backColor(Term::Color::BLUE);
        oddLine = false;
      } else {
        //Term::defaultBackColor();
        oddLine = true;
      }
    }*/
    
    std::fprintf(stream, "%.*s", depth * NAME_INDENT, spaces);
    std::fprintf(stream, "%-*s", NAME_WIDTH - depth * NAME_INDENT, node->name);
    
    std::fprintf(stream, "%-*llu", REST_WIDTH, node->calls);
    
    //not child of root
    if (node->parent->parent) {
      const float avgParentCalls = static_cast<float>(node->calls) / node->parent->calls;
      std::fprintf(stream, "%-*.*g", REST_WIDTH, NUM_PREC, avgParentCalls);
    } else {
      std::fprintf(stream, "%-*llu", REST_WIDTH, node->calls);
    }

    const float totalTime = node->time * 1e-6f;
    std::fprintf(stream, "%-*.*g", REST_WIDTH, NUM_PREC, totalTime);

    if (node->calls) {
      std::fprintf(stream, "%-*.*g", REST_WIDTH, NUM_PREC, totalTime / node->calls);
    } else {
      std::fprintf(stream, "%-*i", REST_WIDTH, 0);
    }
    
    //not child of root
    if (node->parent->parent) {
      const float percent = (static_cast<float>(node->time) / node->parent->time);
      std::fprintf(stream, "%.*g%%", NUM_PREC, percent * 100.0f);
    }
    
    /*
    The Xcode console doesn't support ANSI escape codes
    if (stream == stdout || stream == stderr) {
      Term::defaultBackColor();
    }*/
    
    std::fprintf(stream, "\n");
  } else {
    newDepth = 0;
  }
  
  //copy children into a vector
  std::vector<TreeNode *> children;
  children.reserve(node->children.size());
  for (auto i = node->children.begin(); i != node->children.end(); ++i) {
    children.push_back(&(i->second));
  }
  
  //sort by total time in accending order
  std::sort(children.begin(), children.end(), [](TreeNode *a, TreeNode *b) {
    return a->time > b->time;
  });
  
  for (auto i = children.begin(); i != children.end(); ++i) {
    recFormatInfo(stream, *i, newDepth);
  }
}

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define SCOPE_TIME(name) Profiler CONCAT(profiler_, __COUNTER__) {name}
#define SCOPE_TIME_INFO() Profiler::formatInfo(stdout)

#else

#define SCOPE_TIME(NAME)
#define SCOPE_TIME_INFO()

#endif

#endif
