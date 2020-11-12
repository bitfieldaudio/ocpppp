#pragma once

#include <string_view>
#include <type_traits>

namespace otto::reflect {

  namespace detail {
    constexpr auto test_visitor = [] (std::string_view, auto&) {};
    constexpr auto const_test_visitor = [] (std::string_view, const auto&) {};
  }

  template<typename T>
  struct visitable {};

  template<typename T>
  concept AVisitable = requires(T& t, const T& ct)
  {
    visitable<T>::visit(t, detail::test_visitor);
    visitable<T>::visit(ct, detail::const_test_visitor);
  };

  template<typename T>
  struct remove_member_pointer {
    using type = T;
  };
  template<typename C, typename T>
  struct remove_member_pointer<T C::*> {
    using type = T;
  };

  template<typename T>
  using remove_member_pointer_t = typename remove_member_pointer<T>::type;

  template<typename Struct, auto Struct::*MemPtr, std::size_t Idx>
  struct MemberBase {
    static constexpr std::size_t index = Idx;
    static constexpr std::size_t next = index + 1;
    using value_type = std::remove_pointer_t<decltype(MemPtr)>;
  };

  template<typename T>
  concept StructureTraits = true;

  struct EmptyStructureTraits {
    template<typename Struct, auto Struct::*MemPtr, std::size_t Idx>
    struct Member {};
  };

  template<typename Struct, StructureTraits Tr = EmptyStructureTraits, std::size_t Idx = 0>
  struct structure {};

  template<typename T>
  concept WithStructure = requires
  {
    structure<T>::next;
  };

  template<typename Struct>
  struct StructureBase {
    template<auto Struct::*MemPtr, StructureTraits Traits, std::size_t Idx>
    struct Member : MemberBase<Struct, MemPtr, Idx>, Traits::template Member<Struct, MemPtr, Idx> {
      using Traits::template Member<Struct, MemPtr, Idx>::Member;
    };
  };

  template<typename Struct>
  template<auto Struct::*MemPtr, StructureTraits Traits, std::size_t Idx>
  requires WithStructure<remove_member_pointer_t<decltype(MemPtr)>> //
    struct StructureBase<Struct>::Member<MemPtr, Traits, Idx>
    : MemberBase<Struct, MemPtr, Idx>,
      Traits::template Member<Struct, MemPtr, Idx>,
      structure<remove_member_pointer_t<decltype(MemPtr)>, Traits, Idx + 1> {
    using TraitMem = typename Traits::template Member<Struct, MemPtr, Idx>;
    using Structure = structure<remove_member_pointer_t<decltype(MemPtr)>, Traits, Idx + 1>;

    constexpr Member(auto&&... args) requires std::is_constructible_v<TraitMem, decltype(args)...>&& std::
      is_constructible_v<Structure, decltype(args)...> : TraitMem(args...),
                                                         Structure(args...)
    {}

    using Structure::next;
  };

} // namespace otto::reflect
