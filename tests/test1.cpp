#include <array>

namespace otto::tests {
  struct [[otto::reflect]] S1 {
    int i1;
    int i2;
    [[otto::reflect_skip]] int skip_this;
  };
  struct [[otto::reflect]] S2 {
    float f;
    static int no_static_vars_in_output;
  };
  struct SkipThis {
    int i1;
    int i2;
  };

  struct [[otto::reflect]] Outer {
    int i1;
    int i2;
    struct [[otto::reflect]] Inner {
      int i2;
    };
  };

  template<typename Type, class Class, std::size_t NTTP>
  struct [[otto::reflect]] Templated {
    Type t1;
    int i1;

    struct [[otto::reflect]] Nested {
      std::array<Class, NTTP> arr;
    };

    Nested nested;
  };

  struct [[otto::reflect]] Empty {
  };
} // namespace otto::tests


struct [[otto::reflect]] State {
  /// The filter frequency
  int i1 = 0;
  int i2 = 0;
  struct [[otto::reflect]] SubState {
    int subi1 = 0;
  } substate;
};
