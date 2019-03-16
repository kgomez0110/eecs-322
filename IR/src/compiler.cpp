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

  IR::Program p = IR::parse_file(argv[optind]);

  std::ofstream outputFile;
  outputFile.open("prog.L3");
  for (IR::Function* f : p.functions) {
    outputFile << "define " << f->name << "(";
    for (int ii = 0; ii<f->arguments.size(); ii+=2) {
      if (ii == 0){
        outputFile << f->arguments[ii+1].value;
      }
      else {
        outputFile << ", " << f->arguments[ii+1].value;
      }
    }
    outputFile << "){\n";
    for (IR::Instruction* i : f->instructions){
      outputFile << i->toL3() << "\n";
    }
    outputFile << "}\n";
  }
  outputFile.close();

  return 0;
}