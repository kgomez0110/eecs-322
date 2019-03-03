#pragma once

#include <vector>
#include <set>
#include <utility>

#include <L2.h>

namespace L2{
  struct Graph;
  struct Node;
  Graph generateGraph(std::vector<std::set<Variable*>> in, std::vector<std::set<Variable*>> out, Function &f);
  std::vector<Variable*> coloring (Graph &g, Function &f);

  struct Graph {
    std::unordered_map<Variable*, Node*> nodes;
  };

  struct Node {
    Variable* var;
    std::set<Node*> neighbors;
    int color = 15;
    int degree;
    Node(Variable* v) : var(v) { }
  };


}