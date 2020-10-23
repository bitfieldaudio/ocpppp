namespace otto::tests {
  struct [[otto::reflect]] S {
    int i1;
    int i2;
    [[otto::reflect_skip]] int skip_this;
  };
} // namespace otto::tests
