#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <utility>
#include <iostream>
#include <algorithm>

#include <L2.h>


using namespace std;


namespace L2 {

  int64_t index_instruction(Function f, Instruction* i, int64_t size){
    for (int ii = 0; ii<size; ii++){
      if (f.instructions[ii] == i){
        return ii;
      }
    }
    return 0;
  }
  std::pair<std::vector<std::set<Variable*>>, std::vector<std::set<Variable*>>> liveness_analysis(Function f){
    int64_t num_instructions = f.instructions.size();
    std::vector<std::set<Variable*>> in;
    std::vector<std::set<Variable*>> out;
    std::vector<std::set<Variable*>> old_in;
    std::vector<std::set<Variable*>> old_out;
    std::set<Variable*> kill;
    bool different;
    bool change = true;
    for (int ii = 0; ii<num_instructions; ii++){
      in.push_back({});
      out.push_back({});
    }

    do {
      different = false;
      for (int ii = num_instructions-1; ii>=0; ii++){
        Instruction* instruction = f.instructions[ii];
        in[ii] = instruction->variables_read;
        kill = instruction->variables_killed;
        for (Variable* var : out[ii]){
          if (!kill.count(var)){
            in[ii].insert(var);
          }
        }
        switch (instruction->inst){
          case CJUMP2: {
            // cjump t cmp t label label
            std::string label1 = instruction->operands[4];
            std::string label2 = instruction->operands[5];
            Instruction* first_succ = f.labels[label1];
            Instruction* second_succ = f.labels[label2];
            int64_t first_succ_index = index_instruction(f, first_succ, num_instructions);
            int64_t second_succ_index = index_instruction(f, second_succ, num_instructions);
            out[ii] = in[first_succ_index];
            for (Variable* var : in[second_succ_index]){
              out[ii].insert(var);
            } 
          }
          case CJUMP: {
            // cjump t cmp label
            std::string label1 = instruction->operands[4];
            Instruction* first_succ = f.labels[label1];
            int64_t first_succ_index = index_instruction(f, first_succ, num_instructions);
            int64_t second_succ_index = ii + 1;
            out[ii] = in[first_succ_index];
            for (Variable* var : in[second_succ_index]){
              out[ii].insert(var);
            } 
          }
          case GOTO: {
            // goto label
            std::string label1 = instruction->operands[4];
            Instruction* first_succ = f.labels[label1];
            int64_t first_succ_index = index_instruction(f, first_succ, num_instructions);
            out[ii] = in[first_succ_index];
          }
          case RET: {
            out[ii] = {};
          }
          case LABEL:
          case STACK:
          case OTHER: {
            if (ii != num_instructions-1){
              out[ii] = in[ii+1];
            }
          }
        }
        if (!different){
          different = (old_in[ii] != in[ii]) || (old_out[ii] != out[ii]);
        }
      }
      if (!different){
        change = false;
      }
      old_in = in;
      old_out = out;
    }
    while (change);

    std::pair<std::vector<std::set<Variable*>>, std::vector<std::set<Variable*>>> result = {in, out};
    return result;
  }

  void print_liveness(std::vector<std::set<Variable*>> in, std::vector<std::set<Variable*>> out){
    for (std::set<Variable*> vec : in){
      std::cout << "(";
      for (Variable* var : vec){
        std::cout << var->name << " ";
      }
      std::cout << ")" << std::endl;
    }
    std::cout << ")\n\n(out\n";
    for (std::set<Variable*> vec : out){
      std::cout << "(";
      for (Variable* var : vec){
        std::cout << var->name << " ";
      }
      std::cout << ")\n";
    }
    std::cout << ")\n\n)";
  }





}