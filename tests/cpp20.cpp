
template<typename T>
concept test_cpt = true;

template<test_cpt T>
struct [[otto::reflect]] S {
  int yo;
};
