#pragma once

#include <vector>
#include <set>
#include <utility>

#include <L2.h>

namespace L2 {
  std::pair<std::vector<std::set<Variable*>>, std::vector<std::set<Variable*>>> liveness_analysis(Function f);
  void print_liveness(std::vector<std::set<Variable*>> in, std::vector<std::set<Variable*>> out);
  
}