//
// Created by obhi on 9/17/20.
//
#include <json_bind.hpp>
#include <json_ostream.hpp>
#include <iostream>
#include <vector>
#include <map>

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
      bind("value", &test::value),
      bind("name", &test::name)
      );
}
template <>
auto decl<complex>()
{
  return json_bind(
      bind("array", &complex::array),
      bind("map", &complex::map)
  );
}

}

int main()
{
  test obj;
  obj.value = 1;
  obj.name = "1";

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


  jsb::print_obj(std::cout, obj);
  std::cout << std::endl;
  jsb::print_array(std::cout, complx.array);
  std::cout << std::endl;
  jsb::print_obj(std::cout, complx);
}
