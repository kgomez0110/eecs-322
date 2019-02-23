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
        liveness_only = strtoul(optarg, NULL, 0);
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

    return 0;
  }

  /*
   * Liveness test.
   */
  if (liveness_only){
    //TODO
    L2::Function f = L2::parse_function(argv[optind]);
    std::pair<std::vector<std::set<L2::Variable*>>, std::vector<std::set<L2::Variable*>>> in_out = L2::liveness_analysis(f);
    L2::print_liveness(in_out.first, in_out.second);
    return 0;
  }

  /*
   * Interference graph test.
   */
  if (interference_only){
    // std::vector<std::vector<std::vector<std::string>>> sets = L2::parse_function(argv[optind]);
    // std::vector<std::vector<std::string>> in = sets[0];
    // std::vector<std::vector<std::string>> out = sets[1];
    // std::unordered_map<std::string, std::set<std::string>> graph = L2::generate_graph(in, out, L2::get_kill());
    // std::unordered_map<std::string, std::set<std::string>>::iterator it = graph.begin();
    // while (it != graph.end()){
    //   if (it->first[0] == '%'){
    //     std::cout << it->first.substr(1);
    //   }
    //   else{
    //     std::cout << it->first;
    //   }
    //   std::set<std::string>::iterator its = it->second.begin();
    //   while (its != it->second.end()){
    //     std::string temp = *its;
    //     if (temp[0] == '%'){
    //       std::cout << ' ' << temp.substr(1);
    //     }
    //     else{
    //       std::cout << ' ' << temp;
    //     }
    //     its++;
    //   }
    //   std::cout << '\n';
    //   it++;
    // }
    return 0;
  }

  /*
   * Generate the target code.
   */
  if (enable_code_generator){
    //TODO
  }

  return 0;
}
