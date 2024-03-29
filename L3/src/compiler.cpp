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

  std::vector<std::string> arguments = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

  L3::Program p = L3::parse_file(argv[optind]);
  std::ofstream outputFile;
  outputFile.open("prog.L2");
  outputFile << "(:main\n";
  for (L3::Function* f : p.functions) {
    // function prelogue. making arguments and stuff
    outputFile << "(" + f->name << "\n" << std::to_string(f->arguments.size()) << " 0\n";
    for (int ii = 0; ii<f->arguments.size(); ii++) {
      if (ii < 6) {
        outputFile << f->arguments[ii].value << " <- " << arguments[ii] << "\n";
      }
      else {
        outputFile << f->arguments[ii].value << " <- stack-arg " << std::to_string((ii-6) * 8) << "\n";
      }
    }

    for (L3::Instruction* i : f->instructions) {
      outputFile << i->toL2() << "\n";
    }

    outputFile << ")\n";
  }
  outputFile << ")";
  outputFile.close();
  return 0;
}