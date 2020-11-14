#include "reflect.hpp"
#include <algorithm>
#include "cppast/cpp_template.hpp"
#include "cppast/visitor.hpp"

#include "inja.hpp"

namespace ocpppp::reflect {

  std::string qualified_name(const cppast::cpp_entity& e)
  {
    std::string name = e.parent() ? qualified_name(e.parent().value()) : "";
    if (cppast::is_templated(e)) return name;
    if (auto val = e.scope_name(); val.has_value()) {
      name += "::";
      name += val.value().name();
    } else return name;
    if (cppast::is_template(e.kind())) {
      auto& templ = static_cast<const cppast::cpp_template&>(e);
      name += "<";
      for (auto& param : templ.parameters()) {
        name += param.name();
        name += ", ";
      }
      if (name.ends_with(", ")) name.erase(name.size() - 2, 2);
      name += ">";
    }
    return name;
  }

  std::string all_template_params(const cppast::cpp_entity& e)
  {
    std::string params = e.parent() ? all_template_params(e.parent().value()) : "";
    if (cppast::is_template(e.kind())) {
      auto& templ = static_cast<const cppast::cpp_template&>(e);
      for (auto& param : templ.parameters()) {
        if (param.kind() == cppast::cpp_entity_kind::template_type_parameter_t) {
          auto& tt = static_cast<const cppast::cpp_template_type_parameter&>(param);
          params += tt.keyword_or_constraint().spelling();
        } else if (param.kind() == cppast::cpp_entity_kind::non_type_template_parameter_t) {
          auto& tt = static_cast<const cppast::cpp_non_type_template_parameter&>(param);
          params += to_string(tt.type());
        }
        params += " ";
        params += param.name();
        params += ", ";
      }
    }
    if (params.ends_with(", ")) params.erase(params.size() - 2, 2);
    return params;
  }

  bool is_reflect_struct(const cppast::cpp_entity& e)
  {
    return e.kind() == cppast::cpp_entity_kind::class_t && cppast::has_attribute(e, attribute_name);
  }

  bool should_reflect_member(const cppast::cpp_entity& e)
  {
    if (cppast::has_attribute(e, skip_attribute_name)) return false;
    if (e.kind() == cppast::cpp_entity_kind::member_variable_t) {
      return true;
    }
    return false;
  }

  void gen_structure(std::ostream& output, const cppast::cpp_class& e)
  {
    auto members = nlohmann::json::array();
    for (auto& member : e) {
      if (!should_reflect_member(member)) continue;
      members.push_back({{"name", member.name()}});
    }
    nlohmann::json data = {
      {"params", all_template_params(e)},
      {"qual", qualified_name(e)},
      {"members", members},
    };
    auto templ = R"inja(
// REFLECTION FOR {{qual}}

## for member in members
template<>
struct MemberInfo<&{{qual}}::{{member.name}}> {
  using struct_t = {{qual}};
  using value_type = detail::member_pointer_value_t<decltype(&{{qual}}::{{member.name}})>;
  constexpr static auto mem_ptr = &{{qual}}::{{member.name}};
  constexpr static std::size_t index = {{loop.index}};
  constexpr static util::string_ref name = "{{member.name}}";
};

## endfor
template<StructureTraits Traits, AMemberInfo... Parents{% if (length(params) != 0)%}, {% endif %}{{params}}>
struct structure<{{qual}}, Traits, Parents...> {
## for member in members
  [[no_unique_address]] Member<MemberInfo<&{{qual}}::{{member.name}}>, Traits, Parents...> {{member.name}};
## endfor

## if (length(members) != 0)
  constexpr structure(auto&&... args) requires 
## for member in members
  std::is_constructible_v<decltype({{member.name}}), decltype(args)...> {% if not loop.is_last %}&& //{% endif %}
## endfor
  :  
## for member in members
  {{member.name}}(args...) {% if not loop.is_last %}, //{% endif %}
## endfor
  {}
  constexpr structure(const structure&) = default;
  constexpr structure(structure&&) = default;
  constexpr structure& operator=(const structure&) = default;
  constexpr structure& operator=(structure&&) = default;
## endif
};

template<{{params}}>
struct StructInfo<{{qual}}> {
  using members = meta::list<
## for member in members
    MemberInfo<&{{qual}}::{{member.name}}>{% if not loop.is_last %}, //{% endif %}
## endfor
  >;
};
)inja";
    inja::render_to(output, templ, data);
  }

  void reflect_struct(std::ostream& output, const cppast::cpp_class& e)
  {
    gen_structure(output, e);
  }

  bool process_file(std::ostream& output, const cppast::cpp_file& file, std::string_view input_include)
  {
    std::stringstream contents;
    cppast::visit(
      file, [](const cppast::cpp_entity& e) { return (e.kind() == cppast::cpp_entity_kind::class_t); },
      [&](const cppast::cpp_entity& e, const cppast::visitor_info& info) {
        if (is_reflect_struct(e) && !info.is_old_entity()) {
          auto& struct_ = static_cast<const cppast::cpp_class&>(e);
          reflect_struct(contents, struct_);
        }
      });
    inja::render_to(output, file_template,
                    {
                      {"contents", contents.str()},
                      {"input_include", input_include},
                    });
    return true;
  }
} // namespace ocpppp::reflect
