#pragma once

#include <vector>

namespace L1 {


  /*
   * Instruction interface.
   */
  struct Instruction{
    virtual std::string makeInstruction() = 0;
  };


  /*
   * Instructions.
   */
  struct Instruction_ret : Instruction{
    int64_t arguments, locals;
    std::string makeInstruction() {
      int64_t stack_spaces = locals;
      if (arguments > 6){
        stack_spaces += arguments - 6;
      }
      return "addq $" + std::to_string(stack_spaces * 8) + ", %rsp\nretq\n";
    }
  };

  struct Instruction_mov : Instruction{
    std::string source, destination;
    std::string makeInstruction(){
      return "movq " + source + ", " + destination; 
    }
  };

  struct Instruction_inc : Instruction{
    std::string destination;
    std::string makeInstruction() {
      return "inc " + destination;
    }
  };

  struct Instruction_dec : Instruction {
    std::string destination;
    std::string makeInstruction() {
      return "dec " + destination;
    }
  };
  
  struct Instruction_left_shift : Instruction {
    std::string destination;
    std::string source;
    std::string makeInstruction() {
      return "salq " + source + ", " + destination;
    }
  };

  struct Instruction_right_shift : Instruction {
    std::string destination;
    std::string source;
    std::string makeInstruction() {
      return "sarq " + source + ", " + destination;
    }
  };

  struct Instruction_add : Instruction {
    std::string source;
    std::string destination;
    std::string makeInstruction() {
      return "addq " + source + ", " + destination;
    }
  };

  struct Instruction_sub : Instruction {
    std::string source;
    std::string destination;
    std::string makeInstruction() {
      return "subq " + source + ", " + destination;
    }
  };

  struct Instruction_and : Instruction {
    std::string source;
    std::string destination;
    std::string makeInstruction() {
      return "andq " + source + ", " + destination;
    }
  };

  struct Instruction_mult : Instruction {
    std::string source;
    std::string destination;
    std::string makeInstruction() {
      return "imulq " + source + ", " + destination;
    }
  };


  struct Instruction_jump : Instruction {
    std::string label;
    std::string makeInstruction() {
      return "jmp " + label.substr(1);
    }
  };

  struct Instruction_cjump_two : Instruction {
    std::string t1, t2, cmp, l1, l2;
    bool isFolded, isSwapped; 
    std::string makeInstruction() {
      l1 = l1.substr(1);
      l2 = l2.substr(1);
      if (isFolded){
        if (std::stoll(t1)){
          return "jmp " + l1;
        }
        else{
          return "jmp " + l2;
        }
      }
      else {
        std::string compare = "cmpq " + t2 + ", " + t1 + "\n";
        std::string second_jump = "jmp " + l2;
        std::string first_jump;
        if (cmp == "<="){
          if (isSwapped) {
            first_jump = "jge " + l1 + "\n";
          }
          else{
            first_jump = "jle " + l1 + "\n";
          }
        }
        else if (cmp == "<"){
          if (isSwapped) {
            first_jump = "jg " + l1 + "\n";
          }
          else{
            first_jump = "jl " + l1 + "\n";
          }
        }
        else {
          first_jump = "je " + l1 + "\n";
        }
        return compare + first_jump + second_jump;
      }
   }
  };

  struct Instruction_cjump_one : Instruction {
    std::string t1, t2, cmp, l1;
    bool isSwapped, isFolded;
    std::string makeInstruction() {
      l1 = l1.substr(1);
     if (isFolded){
        if (std::stoll(t1)){
          return "jmp " + l1;
        }
      }
      else {
        std::string compare = "cmpq " + t2 + ", " + t1 + "\n";
        std::string first_jump;
        if (cmp == "<="){
          if (isSwapped) {
            first_jump = "jge " + l1 + "\n";
          }
          else{
            first_jump = "jle " + l1 + "\n";
          }
        }
        else if (cmp == "<"){
          if (isSwapped) {
            first_jump = "jg " + l1 + "\n";
          }
          else{
            first_jump = "jl " + l1 + "\n";
          }
        }
        else {
          first_jump = "je " + l1 + "\n";
        }
        return compare + first_jump;
      } 
    }
  };

  struct Instruction_cmp_code : Instruction {
    std::string destination, t1, t2, cmp, inbetween;
    bool isSwapped, isFolded;
    std::string makeInstruction() {
      if (isFolded){
        return "movq " + t1 + ", " + destination;
      }
      else{
        std::string compare = "cmpq " + t2 + ", " + t1 + "\n";
        std::string move = "movzbq " + inbetween + ", " + destination;
        std::string set;
        if (cmp == "<="){
          if (isSwapped){
            set = "setge " + inbetween + "\n"; 
          }
          else{
            set = "setle " + inbetween + "\n"; 
          }
        }
        else if (cmp == "<"){
          if (isSwapped) {
            set = "setg " + inbetween + "\n";
          }
          else{
            set = "setl " + inbetween + "\n";
          }
        }
        else{
          set = "sete " + inbetween + "\n";
        }
        return compare + set + move;
      }
    }
  };

  struct Instruction_call_runtime : Instruction {
    std::string label;
    std::string makeInstruction() {
      return "call " + label;
    }
  };

  struct Instruction_call : Instruction {
    std::string label;
    int arguments;
    std::string makeInstruction() {
      int stack_spaces;
      if (arguments > 6){
        stack_spaces = (arguments - 6) * 8 + 8; 
      }
      else{
        stack_spaces = 8;
      }
      std::string sub = "subq $" + std::to_string(stack_spaces) + ", %rsp\n";
      if (label[0] == '%'){
        label = "*" + label;
      }
      std::string jump = "jmp " + label;
      return sub + jump;
    }
  };

  struct Instruction_email : Instruction {
    std::string w1, w2, w3, e1;
    std::string makeInstruction() {
      return "lea (" + w2 + ", " + w3 + ", " + e1 + "), " + w1;
    }
  };

  struct Instruction_label : Instruction {
    std::string label;
    std::string makeInstruction() {
      return label.substr(1) + ":";
    }
  };


  /*
   * Function.
   */
  struct Function{
    std::string name;
    int64_t locals;
    int64_t arguments;
    std::vector<Instruction *> instructions;
    std::string makePrologue() {
      return "subq $" + std::to_string(locals * 8) + ", %rsp";
    }
  };

  /*
   * Program.
   */
  struct Program{
    std::string entryPointLabel;
    std::vector<Function *> functions;
  };

}
