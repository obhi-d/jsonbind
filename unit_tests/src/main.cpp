//
// Created by obhi on 9/17/20.
//
#include "json_value.hpp"
#include <cstdlib>
#include <iostream>
#include <json_bind.hpp>
#include <json_ostream.hpp>
#include <json_vstream.hpp>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <vector>
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

struct pointer_test
{
  test*                 basic   = nullptr;
  int*                  nullval = nullptr;
  std::unique_ptr<test> unique;
  std::shared_ptr<test> shared;

  ~pointer_test()
  {
    if (basic)
      delete basic;
  }
};

namespace jsb
{
template <>
auto decl<pointer_test>()
{
  return jsb::json_bind(jsb::bind<&pointer_test::basic>("basic"),
                        jsb::bind<&pointer_test::nullval>("nullval"),
                        jsb::bind<&pointer_test::unique>("unique"),
                        jsb::bind<&pointer_test::shared>("shared"));
}
} // namespace jsb

TEST_CASE("Pointer test", "[validity]")
{
  pointer_test write, read;
  write.basic  = new test;
  write.unique = std::make_unique<test>();
  write.shared = std::make_shared<test>();

  std::stringstream ss;
  jsb::stream_out(ss, write);
  auto jv = nlohmann::json::parse(ss);
  jsb::stream_in(jv, read);
  REQUIRE(read.basic != nullptr);
  REQUIRE(read.unique.get() != nullptr);
  REQUIRE(read.shared.get() != nullptr);
  REQUIRE(*read.basic == *write.basic);
  REQUIRE(read.nullval == write.nullval);
  REQUIRE(*read.unique == *write.unique);
  REQUIRE(*read.shared == *write.shared);
}

struct optional_test
{
  std::optional<int>  set = std::rand();
  std::optional<int>  not_set;
  std::optional<test> set_obj = std::move(test());
  std::optional<test> not_set_obj;

  bool operator==(optional_test const& other) const
  {
    if (set.has_value() != other.set.has_value())
      return false;
    if (not_set.has_value() != other.not_set.has_value())
      return false;
    if (set_obj.has_value() != other.set_obj.has_value())
      return false;
    if (not_set_obj.has_value() != other.not_set_obj.has_value())
      return false;

    if (set.has_value() && *set != *other.set)
      return false;
    if (not_set.has_value() && *not_set != *other.not_set)
      return false;
    if (set_obj.has_value() && *set_obj != *other.set_obj)
      return false;
    if (not_set_obj.has_value() && *not_set_obj != *other.not_set_obj)
      return false;
    return true;
  }
};

namespace jsb
{
template <>
auto decl<optional_test>()
{
  return jsb::json_bind(jsb::bind<&optional_test::set>("set"),
                        jsb::bind<&optional_test::not_set>("not_set"),
                        jsb::bind<&optional_test::set_obj>("set_obj"),
                        jsb::bind<&optional_test::not_set_obj>("not_set_obj"));
}
} // namespace jsb

TEST_CASE("Optional test", "[validity]")
{
  optional_test write, read;

  std::stringstream ss;
  jsb::stream_out(ss, write);

  auto jv = nlohmann::json::parse(ss);
  jsb::stream_in(jv, read);

  REQUIRE(read == write);
}

struct variant_test
{
  std::variant<std::string, int, double, test> one;
  std::variant<std::string, int, double, test> two;
  std::variant<std::string, int, double, test> three;
  std::variant<std::string, int, double, test> four;

  auto operator<=>(variant_test const& other) const = default;
};

namespace jsb
{
template <>
auto decl<variant_test>()
{
  return jsb::json_bind(jsb::bind<&variant_test::one>("one"),
                        jsb::bind<&variant_test::two>("two"),
                        jsb::bind<&variant_test::three>("three"),
                        jsb::bind<&variant_test::four>("four"));
}
} // namespace jsb

TEST_CASE("Variant test", "[validity]")
{
  variant_test write, read;

  write.one   = "string";
  write.two   = std::rand();
  write.three = (double)(int)((float)std::rand() * 10000.0f / (float)RAND_MAX);
  write.four  = test();

  std::stringstream ss;
  jsb::stream_out(ss, write);

  auto jv = nlohmann::json::parse(ss);
  jsb::stream_in(jv, read);

  REQUIRE(read == write);
}

struct int_cast
{
  int value;
};