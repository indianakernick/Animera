//
//  scope time.cpp
//  Animera
//
//  Created by Indiana Kernick on 7/12/19.
//  Copyright © 2019 Indiana Kernick. All rights reserved.
//

#include "scope time.hpp"

#ifdef ENABLE_SCOPE_TIME

#include <vector>
#include <iomanip>
#include <iostream>

void ScopeTime::print() {
  std::cout << std::setw(name_width) << std::left << "Name";
  std::cout << std::setw(rest_width) << std::left << "Total Count";
  std::cout << std::setw(rest_width) << std::left << "Avg Count per parent";
  std::cout << std::setw(rest_width) << std::left << "Total Time (ms)";
  std::cout << std::setw(rest_width) << std::left << "Average Time (ms)";
  std::cout << std::setw(rest_width) << std::left << "Percent of parent time";
  std::cout << '\n';

  const std::size_t prec = std::cout.precision(num_prec);
  printImpl(&tree, 0);
  std::cout.precision(prec);
}

void ScopeTime::reset() {
  current = &tree;
  tree.calls = 0;
  tree.time = {};
  tree.children.clear();
  tree.name = "ROOT";
  tree.parent = nullptr;
}

void ScopeTime::printImpl(const TreeNode *node, const int depth) {
  if (node->name == nullptr) return;
  int newDepth = 0;
  if (node->parent) {
    newDepth = depth + 1;
    
    const int indent = depth * name_indent;
    std::cout << std::setw(indent) << "";
    std::cout << std::setw(name_width - indent) << std::left << node->name;
    
    std::cout << std::setw(rest_width) << std::left << node->calls;
    
    //not child of root
    if (node->parent->parent) {
      const double avg = static_cast<double>(node->calls) / node->parent->calls;
      std::cout << std::setw(rest_width) << std::left << avg;
    } else {
      std::cout << std::setw(rest_width) << std::left << node->calls;
    }

    using MilliFloat = std::chrono::duration<double, std::milli>;
    const double total = std::chrono::duration_cast<MilliFloat>(node->time).count();
    std::cout << std::setw(rest_width) << std::left << total;

    if (node->calls) {
      std::cout << std::setw(rest_width) << std::left << total / node->calls;
    } else {
      std::cout << std::setw(rest_width) << 0;
    }
    
    //not child of root
    if (node->parent->parent) {
      const double percent = (100.0 * node->time.count()) / node->parent->time.count();
      std::cout << percent << '%';
    }
    
    std::cout << '\n';
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

#endif
