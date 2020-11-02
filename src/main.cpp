#include <iostream>
#include <fstream>
#include "command.hpp"
#include "cxxopts.hpp"
#include "reflect/reflect.hpp"

int main(int argc, char* argv[])
{
  cxxopts::Options option_list("ocpppp", "ocpppp - The otto C++ Preprocessor.\n");
  option_list.positional_help("<input file> <output file>");
  // clang-format off
  option_list.add_options()
    ("h,help", "display this help and exit")
    ("version", "display version information and exit")
    ("v,verbose", "be verbose when parsing")
    ("input", "the file that is being parsed", cxxopts::value<std::string>())
    ("output", "the location to place the generated output", cxxopts::value<std::string>())
    ("input-include", "How to include the input file in the output header. Defaults to <input>", cxxopts::value<std::string>()->default_value(""));
  // clang-format on

  auto [file, options] = ocpppp::parse_main(option_list, argc, argv);
  if (!file) return 1;
  std::string input_inc = options["input-include"].as<std::string>();
  if (input_inc.empty()) input_inc = options["input"].as<std::string>();
  if (options.count("output") == 0) {
    ocpppp::reflect::process_file(std::cout, *file, input_inc);
  } else {
    std::ofstream output;
    output.open(options["output"].as<std::string>());
    ocpppp::reflect::process_file(output, *file, input_inc);
  }
}
