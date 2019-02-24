#pragma once

#include <L2.h>

namespace L2{
  Program parse_file (char *fileName);
  Function parse_function (char *fileName);
  std::pair<std::pair<std::string, std::string>, Function> parse_spill_function (char *fileName);
}