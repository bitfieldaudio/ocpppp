#pragma once

#include <memory>
#include "cppast/cpp_file.hpp"
#include "cxxopts.hpp"

namespace ocpppp {

  std::pair<std::unique_ptr<cppast::cpp_file>, cxxopts::ParseResult> parse_main(cxxopts::Options, int argc, char* argv[]);
}
