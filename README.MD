OTTO C++ PREPROCESSOR
=====================

C++ preprocessor and code generator based on [cppast](https://github.com/foonathan/cppast).

## Features

### Reflection

Annotate a type with `[[otto::reflect]]` to generate specializations of `otto::reflect::visitable<T>` and `otto::reflect::structure<T>`.
