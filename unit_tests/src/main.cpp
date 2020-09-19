//
// Created by obhi on 9/17/20.
//
#include "json_value.hpp"
#include <iostream>
#include <json_bind.hpp>
#include <json_ostream.hpp>
#include <json_vstream.hpp>
#include <map>
#include <sstream>
#include <vector>
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this
                          // in one cpp file
#include <catch2/catch.hpp>

struct test
{
  int         value = 0;
  std::string name;

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
  return json_bind(jsb::bind("value", &test::value),
                   jsb::bind("name", &test::name));
}
template <>
auto decl<complex>()
{
  return json_bind(jsb::bind("array", &complex::array),
                   jsb::bind("map", &complex::map));
}
} // namespace jsb
#include <iostream>
template <typename JsonValue, typename Class>
void v_stream_in(JsonValue const& jvalue, Class& obj)
{
  std::cout << typeid(obj).name() << std::endl;
  // json_vstream<JsonValue>(jvalue).stream(obj);
}

TEST_CASE("Simple test", "[validity]")
{
  test outp;
  outp.value = 1;
  outp.name  = "One";

  std::stringstream ss;
  jsb::stream_out(ss, outp);
  auto jv = nlohmann::json::parse(ss);
  REQUIRE(jv.find("name") != jv.end());
  REQUIRE(jv.find("value") != jv.end());

  test inp;
  jsb::stream_in(jv, inp);

  REQUIRE(outp.value == inp.value);
  REQUIRE(outp.name == inp.name);
}

TEST_CASE("Array test", "[validity]")
{
  test obj;
  obj.value = 1;
  obj.name  = "1";
  std::list<test> write;
  write.push_back(obj);
  obj.value = 2;
  obj.name  = "2";
  write.push_back(obj);
  obj.value = 3;
  obj.name  = "3";
  write.push_back(obj);
  obj.value = 4;
  obj.name  = "4";
  write.push_back(obj);

  std::stringstream ss;
  jsb::stream_out(ss, write);
  auto jv = nlohmann::json::parse(ss);

  std::list<test> read;
  jsb::stream_in(jv, read);
  REQUIRE(write == read);
}

TEST_CASE("Nested type test", "[validity]")
{
  test obj;
  obj.value = 1;
  obj.name  = "1";
  complex write, read;
  write.array.push_back(obj);
  obj.value = 2;
  obj.name  = "2";
  write.array.push_back(obj);
  obj.value = 3;
  obj.name  = "3";
  write.array.push_back(obj);
  obj.value = 4;
  obj.name  = "4";
  write.array.push_back(obj);

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
