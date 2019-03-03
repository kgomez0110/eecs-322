#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include <parser.h>
//#include <analysis.h>
//#include <transformer.h>
// #include <code_generator.h>
//#include <spiller.h>
//#include <register_allocation.h>
//#include <utils.h>
#include <liveness.h>
#include <interference.h>
#include <spiller.h>

using namespace std;

void print_help (char *progName){
  std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l 1|2] [-i] SOURCE" << std::endl;
  return ;
}

int main(
  int argc, 
  char **argv
  ){
  auto enable_code_generator = true;
  auto spill_only = false;
  auto interference_only = false;
  int32_t liveness_only = 0;
  int32_t optLevel = 0;

  /* 
   * Check the compiler arguments.
   */
 // Utils::verbose = false;
  if( argc < 2 ) {
    print_help(argv[0]);
    return 1;
  }
  int32_t opt;
  while ((opt = getopt(argc, argv, "vg:O:sl:i")) != -1) {
    switch (opt){

      case 'l':
        // liveness_only = strtoul(optarg, NULL, 0);
        liveness_only = true;
        break ;

      case 'i':
        interference_only = true;
        break ;

      case 's':
        spill_only = true;
        break ;

      case 'O':
        optLevel = strtoul(optarg, NULL, 0);
        break ;

      case 'g':
        enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true ;
        break ;

      case 'v':
     //   Utils::verbose = true; 
        break ;

      default:
        print_help(argv[0]);
        return 1;
    }
  }

  /*
   * Parse the input file.
   */
  if (spill_only){

    /* 
     * Parse an L2 function and the spill arguments.
     */
     //TODO
 
  } else if (liveness_only){

    /*
     * Parse an L2 function.
     */
     //TODO

  } else if (interference_only){

    /*
     * Parse an L2 function.
     */
     //TODO

  } else {

    /* 
     * Parse the L2 program.
     */
    //TODO
  }

  /*
   * Special cases.
   */
  if (spill_only){

    /*
     * Spill.
     */
     //TODO

    std::pair<std::pair<std::string, std::string>, L2::Function> p = L2::parse_spill_function(argv[optind]);
    std::string spill_var = p.first.first;
    std::string prefix = p.first.second;
    L2::Function f = p.second;

    L2::Function newF = L2::spillVar(spill_var, prefix, f);

    std::cout << "(" << newF.name << "\n";
    std::cout << "\t" << std::to_string(newF.arguments) << " " << std::to_string(newF.locals) << "\n";
    for (L2::Instruction* ii : newF.instructions){
      std::cout << "\t";
      if (ii->operands.size() == 2 && (ii->operands[1] == "++" || ii->operands[1] == "--")){
        std::cout << ii->operands[0] << ii->operands[1];
      }
      else{
        std::cout << ii->operands[0];
        for (int jj = 1; jj<ii->operands.size(); jj++){
          std::cout << " " << ii->operands[jj];
        }
      }
      std::cout << "\n";
    }
    std::cout << ")" << std::endl;


    return 0;
  }

  /*
   * Liveness test.
   */
  if (liveness_only){
    //TODO
    L2::Function f = L2::parse_function(argv[optind]);
    // std::string str = "/home/kgu3753/eecs-322/L2/tests/liveness/test4.L2f";
    // char *cstr = new char[str.length() + 1];
    // strcpy(cstr, str.c_str());
    // L2::Function f = L2::parse_function(cstr);
    // delete [] cstr;
    std::pair<std::vector<std::set<L2::Variable*>>, std::vector<std::set<L2::Variable*>>> in_out = L2::liveness_analysis(f);
    std::vector<std::set<L2::Variable*>> in = in_out.first;
    std::vector<std::set<L2::Variable*>> out = in_out.second;
    std::cout << "(" << std::endl << "(in" << std::endl;
    for (std::set<L2::Variable*> vec : in){
      std::cout << "(";
      for (L2::Variable* var : vec){
        if (var->name[0] == '%'){
          std::cout << var->name.substr(1) << " ";
        }
        else{
          std::cout << var->name << " ";
        }
      }
      std::cout << ")" << std::endl;
    }
    std::cout << ")\n\n(out\n";
    for (std::set<L2::Variable*> vec : out){
      std::cout << "(";
      for (L2::Variable* var : vec){
        if (var->name[0] == '%'){
          std::cout << var->name.substr(1) << " ";
        }
        else{
          std::cout << var->name << " ";
        }
      }
      std::cout << ")\n";
    }
    std::cout << ")\n\n)";
    return 0;
  }

  /*
   * Interference graph test.
   */
  if (interference_only){
    L2::Function f = L2::parse_function(argv[optind]);
    // std::string str = "/home/kgu3753/eecs-322/L2/tests/interference/test1.L2f";
    // char *cstr = new char[str.length() + 1];
    // strcpy(cstr, str.c_str());
    // L2::Function f = L2::parse_function(cstr);
    // delete [] cstr;
    std::pair<std::vector<std::set<L2::Variable*>>, std::vector<std::set<L2::Variable*>>> in_out = L2::liveness_analysis(f);
    std::vector<std::set<L2::Variable*>> in = in_out.first;
    std::vector<std::set<L2::Variable*>> out = in_out.second;
    L2::Graph g = L2::generateGraph(in, out, f);
    std::unordered_map<L2::Variable*, L2::Node*>::iterator it = g.nodes.begin();
    while (it != g.nodes.end()){
      if (it->first->name[0] == '%'){
        std::cout << it->first->name.substr(1) << " ";
      }
      else {
        std::cout << it->first->name << " ";
      }
      
      std::set<L2::Node*>::iterator set_it = it->second->neighbors.begin();
      while (set_it != it->second->neighbors.end()){
        if ((*set_it)->var->name[0] == '%'){
          std::cout << (*set_it)->var->name.substr(1) << " ";
        }
        else {
          std::cout << (*set_it)->var->name << " ";
        }
        set_it++;
      }
      std::cout << "\n";
      it++;
    }
    return 0;
  }

  /*
   * Generate the target code.
   */
  if (enable_code_generator){
    //TODO
    L2::Program p = L2::parse_file(argv[optind]);
    // std::string str = "/home/kgu3753/eecs-322/L2/tests/cheungmatt_15540_5156847_hw2_1.L2";
    // char *cstr = new char[str.length() + 1];
    // strcpy(cstr, str.c_str());
    // L2::Program p = L2::parse_file(cstr);
    // delete [] cstr;

    std::vector<std::string> gp_reg = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx", "rsi", "r12", "r13", "r14", "rbp", "rbx", "r15"};
    L2::Program newP;
    newP.entryPointLabel = p.entryPointLabel;
    int count = 0;
    bool all_colored = false;
    L2::Graph g;
    for (L2::Function* f : p.functions){
      L2::Function tempF = *f;
      std::string pre = "%Sha";
      all_colored = false;
      while (!all_colored){
        std::pair<std::vector<std::set<L2::Variable*>>, std::vector<std::set<L2::Variable*>>> in_out = L2::liveness_analysis(tempF);
        std::vector<std::set<L2::Variable*>> in = in_out.first;
        std::vector<std::set<L2::Variable*>> out = in_out.second;
        g = L2::generateGraph(in, out, tempF);
        std::vector<L2::Variable*> needSpilling = L2::coloring(g, tempF);
        if (needSpilling.size() == 0){
          all_colored = true;
        }
        else{
          for (L2::Variable* var : needSpilling){
            std::string prefix = pre + std::to_string(count);
            tempF = L2::spillVar(var->name, prefix, tempF);
            count += 100;
          }
          in_out = L2::liveness_analysis(tempF);
          in = in_out.first;
          out = in_out.second;
          g = L2::generateGraph(in, out, tempF);
          needSpilling = L2::coloring(g, tempF); 
          all_colored = true;
        }
      }
      
      std::unordered_map<std::string, L2::Variable*>::iterator it = tempF.variables.begin();
      while (it != tempF.variables.end()){
        L2::Variable* var = (*it).second;
        for (L2::Instruction* instruct : var->instructions){
          for (int ii = 0; ii<instruct->operands.size(); ii++){
            if (instruct->operands[ii] == var->name){
              if(g.nodes.count(var) && g.nodes[var]->color < 15)
                instruct->operands[ii] = gp_reg[g.nodes[var]->color];
              else
                instruct->operands[ii] = gp_reg[14];
            }
          }
        }
        it++;
      }
      L2::Function* tempF_p = new L2::Function();
      tempF_p->name = tempF.name;
      tempF_p->instructions = tempF.instructions;
      tempF_p->arguments = tempF.arguments;
      tempF_p->locals = tempF.locals;
      newP.functions.push_back(tempF_p);
      count = 0;
    }


      std::ofstream outputFile;
      outputFile.open("prog.L1");

      outputFile << "(" << newP.entryPointLabel << "\n";
      for (L2::Function* f : newP.functions){
        outputFile << "(" << f->name << "\n";
        outputFile << std::to_string(f->arguments) << " " << std::to_string(f->locals) << "\n";
        for (L2::Instruction* i : f->instructions) {
          if (i->inst == L2::STACK){
            outputFile << i->operands[0] << "<- mem rsp " << std::to_string((f->locals * 8) + (std::stoi(i->operands[2])));
          }
          else {
            for (std::string op : i->operands){
              outputFile << op << " ";
            }
          }
          outputFile << "\n";
        }
        outputFile << ")\n";
      }
      outputFile << ")";

      outputFile.close();


  }

  return 0;
}
