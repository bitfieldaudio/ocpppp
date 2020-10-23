#include <iostream>
#include "command.hpp"
#include "reflect/reflect.hpp"

int main(int argc, char* argv[])
{
  auto file = ocpppp::parse_main(argc, argv);
  if (!file) return 1;
  ocpppp::reflect::process_file(std::cout, *file);
}
