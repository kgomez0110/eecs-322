#pragma once

#include <vector>
#include <set>
#include <unordered_map>

namespace LA {
  
  struct Program{
    std::vector<Function*> functions;
  };
  struct Function;
  struct Instruction;
  struct Item;
}