#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <iostream>
#include <fstream>

#include <L3.h>
#include <tiles.h>

namespace L3 {

  void createContexts(Function* f){
    auto c = new Context();
    int count = 0;
    int num_instructions = f->instructions.size();
    for (int jj = 0; jj<num_instructions; jj++){
      auto inst = f->instructions[jj];
      if (inst->type == LABEL && !c->instructions.empty()){
        c->num_instructions = count;
        count = 0;
        f->contexts.push_back(c);
        c = new Context();
      }
      c->instructions.push_back(inst);
      count++;
      if (inst->type == BR || inst->type == BR_VAR){
        c->num_instructions = count;
        count = 0;
        f->contexts.push_back(c);
        c = new Context();
      }
    }
    f->contexts.push_back(c);
    for (int ii = 0; ii<f->contexts.size(); ii++){
      if (f->contexts[ii]->instructions.empty()){
        f->contexts.erase(f->contexts.begin() + ii);
      }
    }
  }


  std::vector<Tile*> generateTiles(Function* f){
    createContexts(f);
    std::vector<Context*> contexts = f->contexts;
    std::vector<Tile*> function_tiles;
    for (Context* c : contexts) {
      for (Instruction* i : c->instructions) {
        function_tiles.push_back(new Tile(i->tree));
      }
    }
    return function_tiles;
  }

  void codeGenerator(Program &p) {
    std::ofstream outputFile;
    outputFile.open("prog.L2");
    outputFile << "(:main\n";
    for (Function* f : p.functions){
      outputFile << "(" << f->name << "\n";
      outputFile << std::to_string(f->arguments.size()) << " 0\n";
      for (int ii = 0; ii<f->arguments.size(); ii++){
        if (ii < 6){
          outputFile << f->arguments[ii].value << " <- " << arguments[ii] << "\n";
        }
        else {
          outputFile << f->arguments[ii].value << "stack-arg " << std::to_string((ii - 6) * 8) << "\n";
        }
      }

      std::vector<Tile*> tiles = generateTiles(f);
      for (Tile* t : tiles){
        outputFile << t->toL2() << "\n";
        
      }
      outputFile << ")\n";
    }
    outputFile << ")";
    outputFile.close();

  }



  
  
}