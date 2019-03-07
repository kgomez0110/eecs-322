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
    for (int ii = 0; ii<f->contexts.size(); ii++){
      if (f->contexts[ii]->instructions.empty()){
        f->contexts.erase(f->contexts.begin() + ii);
      }
    }
  }

  void addEachTile(std::vector<Tile*> &tiles){
    tiles.push_back(new ReturnTileT());
    tiles.push_back(new ReturnTile());
    tiles.push_back(new MoveTile());
    tiles.push_back(new CalleeTile());
    tiles.push_back(new SaveCalleeTile());
    tiles.push_back(new GotoTile());
    tiles.push_back(new CjumpTile());
    tiles.push_back(new LoadTile());
    tiles.push_back(new StoreTile());
    tiles.push_back(new MultiplyTile());
    tiles.push_back(new AddTile());
    tiles.push_back(new SubtractTile());
    tiles.push_back(new AndTile());
    tiles.push_back(new LessThanEqualTile());
    tiles.push_back(new LessThanTile());
    tiles.push_back(new EqualTile());
    tiles.push_back(new GreaterThanEqualTile());
    tiles.push_back(new GreaterThanTile()); 
  }


  std::vector<Tile*> generateTiles(Function* f){
    createContexts(f);
    std::vector<Context*> contexts = f->contexts;
    std::vector<Tile*> tiles;
    addEachTile(tiles);
    std::vector<Tile*> function_tiles;
    int num_tiles = tiles.size();
    for (Context* c : contexts) {
      for (Instruction* i : c->instructions) {
        for (int jj = 0; jj < num_tiles; jj++){
          if (tiles[jj]->match(i->tree)) {
            function_tiles.push_back(tiles[jj]);
            if (jj == 0)
              tiles[jj] = new ReturnTileT();
            else if (jj == 1)
              tiles[jj] = new ReturnTile();
            else if (jj == 2)
              tiles[jj] = new MoveTile();
            else if (jj == 3)
              tiles[jj] = new CalleeTile();
            else if (jj == 4)
              tiles[jj] = new SaveCalleeTile();
            else if (jj == 5)
              tiles[jj] = new GotoTile();
            else if (jj == 6)
              tiles[jj] = new CjumpTile();
            else if (jj == 7)
              tiles[jj] = new LoadTile();
            else if (jj == 8)
              tiles[jj] = new StoreTile();
            else if (jj == 9)
              tiles[jj] = new MultiplyTile();
            else if (jj == 10)
              tiles[jj] = new AddTile();
            else if (jj == 11)
              tiles[jj] = new LessThanEqualTile();
            else if (jj == 12)
              tiles[jj] = new LessThanTile();
            else if (jj == 13)
              tiles[jj] = new EqualTile();
            else if (jj == 14)
              tiles[jj] = new GreaterThanEqualTile();
            else if (jj == 15)
              tiles[jj] = new GreaterThanTile();
            break;
          }
        }
      }
      for (int ii = 0; ii<num_tiles; ii++){
        delete tiles[ii];
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