//
// Created by obhi on 9/18/20.
//
#pragma once

#include <json_bind.hpp>

namespace jsb
{
// json_value from a given API is transformed into
// a bound class using this interface
template <typename Class, typename JsonValue>
requires(detail::BoundClass<Class>) class json_vstream
{
public:
  using json_value = JsonValue;
  class array;
  class object;

  array  as_array();
  object as_object();

  template <typename Value>
  inline void stream(Value& obj);

  json_vstream(json_ostream const&) noexcept = delete;

protected:
  explicit json_vstream(json_value& i_val) noexcept : value(i_val) {}
  json_ostream(json_ostream&& i_other) noexcept : value(i_other.value) {}

protected:
  json_value& ostr;
};

class json_vstream::object : public json_vstream
{
};
} // namespace jsb