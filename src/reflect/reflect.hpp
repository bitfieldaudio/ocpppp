#pragma once

#include "cppast/cpp_file.hpp"

namespace ocpppp::reflect {
  inline const std::string attribute_name = "otto::reflect";
  inline const std::string skip_attribute_name = "otto::reflect_skip";

  inline const std::string file_template = R"inja(#pragma once

#include "lib/reflect.hpp"
#include "{{input_include}}"

namespace otto::reflect {
{{contents}}
}
)inja";

  bool process_file(std::ostream& output, const cppast::cpp_file& file, std::string_view input_include);
}
