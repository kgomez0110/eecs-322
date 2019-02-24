#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <utility>
#include <iostream>
#include <algorithm>

#include <L2.h>
#include <interference.h>


using namespace std;


namespace L2 {

  Variable* createVar(std::string spillPrefix, int64_t count){
    auto var = new Variable(spillPrefix + std::to_string(count));
    return var;
  }

  void replaceVars(Instruction* instruction, Variable* var, Variable* newVar){
    int64_t num_operands = instruction->operands.size();
    for (int ii = 0; ii < num_operands; ii++){
      if (instruction->operands[ii] == var->name){
        instruction->operands[ii] = newVar->name;
      }
    }
  }

  void replaceInstruction(Instruction* instruction, Variable* var, Variable* newVar, Function &f, int64_t &count){
    bool spilled = false;
    if (instruction->variables_read.count(var)){
      std::vector<std::string> newOperands = {newVar->name, "<-", "mem", "rsp", std::to_string(f.locals*8)};
      auto i = new Instruction(newOperands, L2::OTHER);
      f.instructions.push_back(i);
      spilled = true;
      instruction->variables_read.erase(var);
    }
    if (instruction->variables_killed.count(var)){
      instruction->variables_killed.erase(var);
      replaceVars(instruction, var, newVar);
      f.instructions.push_back(instruction);
      std::vector<std::string> newOperands = {"mem", "rsp", std::to_string(f.locals*8), "<-", newVar->name};
      auto i = new Instruction(newOperands, L2::OTHER);
      f.instructions.push_back(i); 
      spilled = true;
    }
    else{
      replaceVars(instruction, var, newVar);
      f.instructions.push_back(instruction);
    }
    if (spilled){
      count++;
    }
    else{
      delete newVar;
    }
  }
  Function spillVar(std::string varName, std::string spillPrefix, Function f){
    if (!f.variables.count(varName)){
      return f;
    }
    Variable* var = f.variables[varName];
    Variable* newVar;
    int64_t count = 0;
    Function newF;
    newF.arguments = f.arguments;
    newF.name = f.name;
    newF.labels = f.labels;
    newF.locals = f.locals;
    newF.variables = f.variables;
    for (Instruction* instruct : f.instructions){
      newVar = createVar(spillPrefix, count);
      replaceInstruction(instruct, var, newVar, newF, count);
    }
    newF.variables.erase(varName);
    newF.locals++;
    return newF;
  }







}