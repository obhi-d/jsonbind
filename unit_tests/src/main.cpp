//
// Created by obhi on 9/17/20.
//
#include <json_bind.hpp>
#include <json_ostream.hpp>
#include <json_vstream.hpp>
#include "json_value.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

struct test
{
  int value;
  std::string name;
};

struct complex
{
  std::vector<test> array;
  std::map<std::string, int> map;
};

namespace jsb
{
template <>
auto decl<test>()
{
  return json_bind(
      jsb::bind("value", &test::value),
      jsb::bind("name", &test::name)
      );
}
template <>
auto decl<complex>()
{
  return json_bind(
      jsb::bind("array", &complex::array),
      jsb::bind("map", &complex::map)
  );
}
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
/*
  complex complx;
  complx.array.push_back(obj);
  obj.value = 2;
  obj.name = "2";
  complx.array.push_back(obj);
  obj.value = 3;
  obj.name = "3";
  complx.array.push_back(obj);
  obj.value = 4;
  obj.name = "4";
  complx.array.push_back(obj);

  complx.map = {
      {"first", 1},
      { "second", 2},
      { "third", 3},
  };

  std::stringstream ss;
  jsb::stream_out(ss, obj);

  std::cout << std::endl;
  jsb::stream_out(ss, complx.array);
  std::cout << std::endl;
  jsb::stream_out(ss, complx);

}
*/