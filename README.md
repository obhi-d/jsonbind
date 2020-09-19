# Introduction

This project aims at supporting json streaming from C++ by using reflection like interface. 
The project borrows ideas from MetaStuff (https://github.com/eliasdaler/MetaStuff.git) and uses C++20.

# Features

- This lib is still in development, so things might change in the future, and not everything is tested.
- Json streaming is directly supported by jsb::json_ostream (for output) and jsb::json_vstream (for input) interface. 
- Any supported json library can be used along with jsb::json_vstream by providing an interface for the json_value type.
- The json_value type interface depends on free function declaration inside namespace jsb. Example can be found in:
unit_tests/src/json_value.hpp
- Container types are directly supported (vector, list, map, etc.). Container type deductions are done by using concepts.

# Dependency
For building tests:
- Catch2
- nlohmann/json

Both modules are fetched by ExternalProject_Add in cmake.

# Usage

Bindings are expected to be declared inside namespace jsb. Example binding is given.

```cpp
struct test
{
  int         value  = 0;
  float       fvalue = 0;
  bool        bvalue = 0;
  unsigned    uvalue = 0;
  std::string name;

  test()
  {
    value  = std::rand();
    fvalue = (float)(int)((float)std::rand() * 10000.0f / (float)RAND_MAX);
    bvalue = (float)std::rand() / (float)RAND_MAX > 0.5f;
    uvalue = (unsigned)std::rand();
    name   = "SuffixNum" + std::to_string(std::rand());
  }

  float get_fvalue() const
  {
    return (float)(int)fvalue;
  }
  void set_fvalue(float v)
  {
    fvalue = v;
  }
  auto operator<=>(test const&) const = default;
};

struct complex
{
  std::vector<test>          array;
  std::map<std::string, int> map;
  auto                       operator<=>(complex const&) const = default;
};

namespace jsb
{
template <>
auto decl<test>()
{
  return json_bind(jsb::bind<&test::value>("value"),
                   jsb::bind<&test::get_fvalue, &test::set_fvalue>("fvalue"),
                   jsb::bind<&test::bvalue>("bvalue"),
                   jsb::bind<&test::uvalue>("uvalue"),
                   jsb::bind<&test::name>("name"));
}
template <>
auto decl<complex>()
{
  return json_bind(jsb::bind<&complex::array>("array"),
                   jsb::bind<&complex::map>("map"));
}
} // namespace jsb
```

Please check the unit_tests folder for examples.

# Bulding
jsonbind is header only. However, it is not a single header library.
- json_bind.hpp : Declares the necessary decl and bind functions.
- json_ostream.hpp : Outputs json from class interface.
- json_vstream.hpp : Can stream in a json_value type to an appropriate container/object

