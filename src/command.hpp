#pragma once

#include <memory>
#include "cppast/cpp_file.hpp"

namespace ocpppp {
  std::unique_ptr<cppast::cpp_file> parse_main(int argc, char* argv[]);
}
