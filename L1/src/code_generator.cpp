#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

using namespace std;

namespace L1{
  void generate_code(Program p){

    /* 
     * Open the output file.
     */ 
    std::ofstream outputFile;
    outputFile.open("prog.S");
   
    /* 
     * Generate target code
     */ 
    //TODO

    outputFile << " .text\n.globl go\ngo:\npushq %rbx\npushq %rbp\npushq %r12\npushq %r13\npushq %r14\npushq %r15\n";
    outputFile << "call " + p.entryPointLabel + "\n";
    outputFile << "popq %r15\npopq %r14\npopq %r13\npopq %r12\npopq %rbp\npopq %rbx\nretq\n";
    int64_t num_functions = p.functions.size();
    for (int ii = 0; ii<num_functions; ii++){
      auto currentF = p.functions[ii];
      outputFile << currentF->name + ":\n"; 
      outputFile << currentF->makePrologue() << '\n';
      auto instruct = currentF->instructions;
      int64_t num_instructions = instruct.size();
      for (int jj = 0; jj<num_instructions; jj++){
        outputFile << instruct[jj]->makeInstruction() << '\n';
      }
    } 

    /* 
     * Close the output file.
     */ 
    outputFile.close();
   
    return ;
  }
}
