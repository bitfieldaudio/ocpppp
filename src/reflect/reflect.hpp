#pragma once

#include "cppast/cpp_file.hpp"

namespace ocpppp::reflect {
  inline const std::string attribute_name = "otto::reflect";
  inline const std::string skip_attribute_name = "otto::reflect_skip";
  bool process_file(std::ostream& output, const cppast::cpp_file& file);
}
