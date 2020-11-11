#include "reflect.hpp"
#include <algorithm>
#include "cppast/cpp_template.hpp"
#include "cppast/visitor.hpp"

#include "inja.hpp"

namespace ocpppp::reflect {

  std::vector<std::string> namespaces;

  std::string qualified_name(const cppast::cpp_entity& e)
  {
    std::vector<std::string> scopes;
    auto* parent = &e;
    do {
      if (cppast::is_templated(*parent)) continue;
      std::string scope = "";
      if (auto val = parent->scope_name(); val.has_value()) {
        scope += val.value().name();
      } else
        continue;
      if (cppast::is_template(parent->kind())) {
        auto& templ = static_cast<const cppast::cpp_template&>(*parent);
        scope += "<";
        for (auto& param : templ.parameters()) {
          scope += param.name();
          scope += ", ";
        }
        if (scope.ends_with(", ")) scope.erase(scope.size() - 2, 2);
        scope += ">";
      }
      scopes.push_back(scope);
    } while (parent->parent() && (parent = &parent->parent().value()));
    std::string name = "";
    for (int i = scopes.size() - 1; i >= 0; i--) {
      name += "::";
      name += scopes[i];
    }
    return name;
  }

  std::string all_template_params(const cppast::cpp_entity& e)
  {
    std::vector<std::string> params;
    auto* parent = &e;
    do {
      if (cppast::is_template(parent->kind())) {
        auto& templ = static_cast<const cppast::cpp_template&>(*parent);
        std::string res;
        for (auto& param : templ.parameters()) {
          if (param.kind() == cppast::cpp_entity_kind::template_type_parameter_t) {
            auto& tt = static_cast<const cppast::cpp_template_type_parameter&>(param);
            res += tt.keyword_or_constraint().spelling();
          } else if (param.kind() == cppast::cpp_entity_kind::non_type_template_parameter_t) {
            auto& tt = static_cast<const cppast::cpp_non_type_template_parameter&>(param);
            res += to_string(tt.type());
          }
          res += " ";
          res += param.name();
          res += ", ";
        }
        params.push_back(res);
      }
    } while (parent->parent() && (parent = &parent->parent().value()));
    std::string res = "";
    for (int i = params.size() - 1; i >= 0; i--) {
      res += params[i];
    }
    if (res.ends_with(", ")) res.erase(res.size() - 2, 2);
    return res;
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

  void gen_visitable(std::ostream& output, const cppast::cpp_class& e)
  {
    std::vector<std::string> members;
    for (auto& member : e) {
      if (!should_reflect_member(member)) continue;
      members.push_back(member.name());
    }
    nlohmann::json data = {
      {"params", all_template_params(e)},
      {"qual", qualified_name(e)},
      {"members", members},
    };
    auto templ = R"(
template<{{params}}>
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

  void gen_structure(std::ostream& output, const cppast::cpp_class& e)
  {
    auto members = nlohmann::json::array();
    std::string idx = "0";
    for (auto& member : e) {
      if (!should_reflect_member(member)) continue;
      members.push_back({{"name", member.name()}, {"idx", idx}});
      idx = "decltype(" + member.name() + ")::next";
    }
    nlohmann::json data = {
      {"params", all_template_params(e)},
      {"qual", qualified_name(e)},
      {"members", members},
      {"last_idx", idx},
    };
    auto templ = R"inja(
template<StructureTraits Tr, std::size_t Idx{% if (length(params) != 0)%}, {% endif %}{{params}}>
struct structure<{{qual}}, Tr, Idx> : StructureBase<{{qual}}> {
## for member in members
  [[no_unique_address]] Member<&{{qual}}::{{member.name}}, Tr, Idx + {{member.idx}}> {{member.name}};
## endfor
  static constexpr std::size_t next = Idx + {{last_idx}};

## if (length(members) != 0)
  constexpr structure(auto&&... args) requires 
## for member in members
  std::is_constructible_v<decltype({{member.name}}), decltype(args)...> {% if not loop.is_last %}&&{% endif %}
## endfor
  :  
## for member in members
  {{member.name}}(args...) {% if not loop.is_last %},{% endif %}
## endfor
  {}
  constexpr structure(const structure&) = default;
  constexpr structure(structure&&) = default;
  constexpr structure& operator=(const structure&) = default;
  constexpr structure& operator=(structure&&) = default;
## endif
};
)inja";
    inja::render_to(output, templ, data);
  }

  void reflect_struct(std::ostream& output, const cppast::cpp_class& e)
  {
    gen_visitable(output, e);
    gen_structure(output, e);
  }

  bool process_file(std::ostream& output, const cppast::cpp_file& file, std::string_view input_include)
  {
    std::stringstream contents;
    cppast::visit(
      file,
      [](const cppast::cpp_entity& e) {
        return (e.kind() == cppast::cpp_entity_kind::class_t || e.kind() == cppast::cpp_entity_kind::namespace_t);
      },
      [&](const cppast::cpp_entity& e, const cppast::visitor_info& info) {
        if (is_reflect_struct(e) && !info.is_old_entity()) {
          auto& struct_ = static_cast<const cppast::cpp_class&>(e);
          reflect_struct(contents, struct_);
        } else if (e.kind() == cppast::cpp_entity_kind::namespace_t) {
          if (info.event == cppast::visitor_info::container_entity_enter)
            namespaces.push_back(e.name());
          else // if (info.event == cppast::visitor_info::container_entity_exit)
            namespaces.pop_back();
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
