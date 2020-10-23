#include "reflect.hpp"
#include <algorithm>
#include "cppast/visitor.hpp"

#include "inja.hpp"

namespace ocpppp::reflect {

  std::vector<std::string> namespaces;

  std::string qualified_name(const cppast::cpp_entity& e)
  {
    std::string name = "::";
    for (auto& n : namespaces) {
      name += n += "::";
    }
    name += e.name();
    return name;
  }

  bool is_reflect_struct(const cppast::cpp_entity& e)
  {
    return e.kind() == cppast::cpp_entity_kind::class_t && cppast::has_attribute(e, attribute_name);
  }

  void reflect_struct(std::ostream& output, const cppast::cpp_class& e)
  {
    std::vector<std::string> members;
    for (auto& member : e) {
      if (cppast::has_attribute(member, skip_attribute_name)) continue;
      members.push_back(member.name());
    }
    nlohmann::json data = {
      {"qual", qualified_name(e)},
      {"members", members},
    };
    auto templ = R"(
template<>
struct visitable<{{ qual }}> {
  static void visit({{ qual }}& self, auto&& visitor) {
## for member in members
    visitor("{{ member }}", self.{{ member }});
## endfor
  }
  static void visit(const {{ qual }}& self, auto&& visitor) {
## for member in members
    visitor("{{ member }}", self.{{ member }});
## endfor
  }
};
)";
    inja::render_to(output, templ, data);
  }

  bool process_file(std::ostream& output, const cppast::cpp_file& file)
  {
    cppast::visit(
      file,
      [](const cppast::cpp_entity& e) {
        return (e.kind() == cppast::cpp_entity_kind::class_t || e.kind() == cppast::cpp_entity_kind::namespace_t);
      },
      [&](const cppast::cpp_entity& e, const cppast::visitor_info& info) {
        if (is_reflect_struct(e) && !info.is_old_entity()) {
          auto& struct_ = static_cast<const cppast::cpp_class&>(e);
          reflect_struct(output, struct_);
        } else if (e.kind() == cppast::cpp_entity_kind::namespace_t) {
          if (info.event == cppast::visitor_info::container_entity_enter)
            namespaces.push_back(e.name());
          else if (info.event == cppast::visitor_info::container_entity_exit)
            namespaces.pop_back();
        }
      });
    return true;
  }
} // namespace ocpppp::reflect
