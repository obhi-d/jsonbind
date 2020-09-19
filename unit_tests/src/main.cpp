//
// Created by obhi on 9/17/20.
//
#include "json_value.hpp"
#include <cstdlib>
#include <iostream>
#include <json_bind.hpp>
#include <json_ostream.hpp>
#include <json_vstream.hpp>
#include <map>
#include <sstream>
#include <vector>
#include <list>
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in one cpp file
#include <catch2/catch.hpp>

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

TEST_CASE("Simple test", "[validity]")
{
  test outp;

  std::stringstream ss;
  jsb::stream_out(ss, outp);
  auto jv = nlohmann::json::parse(ss);

  test inp;
  jsb::stream_in(jv, inp);

  REQUIRE(outp == inp);
}

TEST_CASE("Array test", "[validity]")
{
  std::list<test> write;
  write.push_back(test());
  write.push_back(test());
  write.push_back(test());
  write.push_back(test());

  std::stringstream ss;
  jsb::stream_out(ss, write);
  auto jv = nlohmann::json::parse(ss);

  std::list<test> read;
  jsb::stream_in(jv, read);
  REQUIRE(write == read);
}

TEST_CASE("Nested type test", "[validity]")
{
  complex write, read;
  write.array.push_back(test());
  write.array.push_back(test());
  write.array.push_back(test());
  write.array.push_back(test());

  write.map = {
      {"first", 1},
      {"second", 2},
      {"third", 3},
  };

  std::stringstream ss;
  jsb::stream_out(ss, write);
  auto jv = nlohmann::json::parse(ss);
  jsb::stream_in(jv, read);
  REQUIRE(write == read);
}
