#pragma once

#include <vector>
#include <set>
#include <utility>

#include <L2.h>

namespace L2{
  struct Graph;
  struct Node;
  void addEdge(Variable* v1, Variable* v2, Graph& g);
  void addEdgesSet(std::set<Variable*> vars, Graph& g);
  void addGPs(std::vector<std::string> gp, Graph& g);
  void addEdgesBetweenSets(std::set<Variable*> s1, std::set<Variable*> s2, Graph &g);
  Graph generateGraph(std::vector<std::set<Variable*>> in, std::vector<std::set<Variable*>> out, Function f);
  Variable* getVarFromSet(std::set<Variable*> s, int64_t index);

  struct Graph {
    std::unordered_map<Variable*, Node*> nodes;
  };

  struct Node {
    Variable* var;
    std::set<Node*> neighbors;
    Node(Variable* v) : var(v) { }
  };


}