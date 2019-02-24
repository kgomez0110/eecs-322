#pragma once

#include <vector>
#include <set>
#include <unordered_map>

namespace L2 {
  
  enum Special_inst{
    RET,
    CJUMP2,
    CJUMP,
    GOTO,
    LABEL,
    STACK,
    OTHER,
    SOPSX
  };
  struct Instruction;
  
  struct Variable {
    std::string name;
    std::vector<Instruction*> instructions;
    Variable(std::string n) : name(n) { }
  };


  struct Instruction {
    std::set<Variable*> variables_read;
    std::set<Variable*> variables_killed;
    std::vector<std::string> operands;
    Special_inst inst;
    Instruction(std::vector<std::string> op, Special_inst i) :
      operands(op), inst(i) { }
  };


  struct Function {
    std::string name;
    std::unordered_map<std::string, Instruction*> labels;
    std::unordered_map<std::string, Variable*> variables;
    std::vector<Instruction*> instructions;
    int64_t arguments;
    int64_t locals;
  };

  struct Program{
    std::string entryPointLabel;
    std::vector<Function *> functions;
  };

}