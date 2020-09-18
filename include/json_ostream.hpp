//
// Created by obhi on 9/17/20.
//
#pragma once

#include <ostream>
#include <cassert>
#include <string>
#include <string_view>
#include <json_bind.hpp>

namespace jsb
{


class json_ostream
{
public:
  struct array;
  struct object;

  array         as_array();
  object        as_object();

  template <typename Value>
  inline void print(Value const& obj);

  json_ostream(json_ostream const&) noexcept = delete;

protected:


  explicit json_ostream(std::ostream& i_ostr) noexcept : ostr(i_ostr) {} 
  json_ostream(json_ostream&& i_other) noexcept
      : ostr(i_other.ostr)
  {
  }


protected:
  std::ostream& ostr;
};

class json_ostream::object : public json_ostream
{
public:
  explicit object(std::ostream& ostr) : json_ostream(ostr) { begin(); }
  object(object&& i_other) noexcept
      : json_ostream(std::move(i_other)), first(i_other.first)
  {
    i_other.moved = true;
  }

  object(object const& ostr) noexcept = delete;
  ~object() { end(); }

  template <typename Class>
  requires (detail::is_class_bound<Class>)
  void print(Class const& obj)
  {
    detail::for_all<Class>(*this, obj);
  }

  template <typename Class>
  requires (detail::is_name_pair_list<Class>)
  void print(Class const& obj)
  {
    for (auto& pair : obj)
    {
      if (!first)
        ostr << ", ";
      first = false;
      ostr << "\"" << detail::as_string(pair.first) << "\": ";
      json_ostream::print(pair.second);
    }
  }

  template <typename Class, typename Decl>
  inline void operator() (Class const& obj, Decl const& decl);

private:
  void begin()
  {
    ostr << "{ ";
  }
  void end()
  {
    if (!moved)
      ostr << " }";
  }
  bool first = true;
  bool moved = false;
};

class json_ostream::array : public json_ostream
{
public:
  array(array&& i_other) noexcept
      : json_ostream(std::move(i_other)), first(i_other.first)
  {
    i_other.moved = true;
  }
  array(array const& ostr) noexcept = delete;
  explicit array(std::ostream& ostr) : json_ostream(ostr) { begin(); }
  ~array() { end(); }

  template <typename Class>
  void print(Class const& obj)
  {
    for(auto const& each : obj)
    {
      using value_type = std::decay_t<decltype(each)>;
      if (!first)
        ostr << ", ";
      first = false;
      json_ostream::print(each);
    }
  }

private:
  void begin()
  {
    ostr << "[ ";
  }
  void end()
  {
    if (!moved)
      ostr << " ]";
  }
  bool first = true;
  bool moved = false;
};

json_ostream::array json_ostream::as_array()
{
  return json_ostream::array(ostr);
}

json_ostream::object json_ostream::as_object()
{
  return json_ostream::object(ostr);
}

template <typename Value>
void json_ostream::print(const Value& obj)
{
  using value_type = std::decay_t<Value>;

  if constexpr (detail::is_class_bound<value_type>)
    json_ostream::object(ostr).print(obj);
  else if constexpr (detail::is_bool_v<value_type>)
    ostr << std::boolalpha << static_cast<bool>(obj);
  else if constexpr (detail::is_float_v<value_type>)
    ostr << static_cast<double>(obj);
  else if constexpr (detail::is_signed_v<value_type>)
    ostr << static_cast<std::int64_t>(obj);
  else if constexpr (detail::is_unsigned_v<value_type>)
    ostr << static_cast<std::uint64_t>(obj);
  else if constexpr (detail::is_string_v<value_type>)
    ostr << "\"" << detail::as_string(obj) << "\"";

}

template <typename Class>
void print_obj(std::ostream& ostr, Class const& obj)
{
  json_ostream::object printer(ostr);
  printer.print(obj);
}

template <typename Class>
void print_array(std::ostream& ostr, Class const& obj)
{
  json_ostream::array printer(ostr);
  printer.print(obj);
}

template <typename Class, typename Decl>
void json_ostream::object::operator()(Class const& obj, const Decl& decl)
{
  if (!first)
    ostr << ", ";
  first = false;

  ostr << "\"" << decl.key() << "\": ";
  json_ostream::print(decl.value(obj));
}

}