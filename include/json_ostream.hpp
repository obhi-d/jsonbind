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

  template <typename T>
  inline json_ostream& operator=(T const& iVal)
  {
    ostr << iVal;
    return *this;
  }

  inline json_ostream& operator=(std::string_view const& iVal)
  {
    ostr << "\"" << iVal << "\"";
    return *this;
  }
  inline json_ostream& operator=(std::string const& iVal)
  {
    ostr << "\"" << iVal << "\"";
    return *this;
  }
  inline json_ostream& operator=(char const* iVal)
  {
    ostr << "\"" << iVal << "\"";
    return *this;
  }
  inline json_ostream& operator=(bool iVal)
  {
    ostr << std::boolalpha << iVal;
    return *this;
  }

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
  requires (tuple_size<Class> > 0)
  void print(Class const& obj)
  {
    detail::for_all<Class>(*this, obj);
  }

  template <typename Class>
  requires (is_named_map_type<Class>)
  void print(Class const& obj)
  {
    for (auto& pair : obj)
    {
      if (!first)
        ostr << ", ";
      first = false;
      ostr << "\"" << pair.first << "\": ";
      using value_type = std::decay_t<decltype(pair.second)>;

      if constexpr (tuple_size<value_type> > 0)
        json_ostream::object(ostr).print(pair.second);
      else if constexpr (std::is_same_v<value_type, bool>)
        ostr << std::boolalpha << static_cast<bool>(pair.second);
      else if constexpr (std::is_same_v<value_type, float> ||
                         std::is_same_v<value_type, double>)
        ostr << static_cast<double>(pair.second);
      else if constexpr (std::is_signed_v<value_type> &&
                         std::is_convertible_v<value_type, std::int64_t>)
        ostr << static_cast<std::int64_t>(pair.second);
      else if constexpr (std::is_unsigned_v<value_type> &&
                         std::is_convertible_v<value_type, std::uint64_t>)
        ostr << static_cast<std::uint64_t>(pair.second);
      else
        ostr << "\"" << std::string_view(pair.second) << "\"";

    }
    detail::for_all<Class>(*this, obj);
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

  template <typename T>
  inline array& operator=(T const& iIterable)
  {
    for (auto const& val : iIterable)
    {
      if (!first)
        ostr << ", ";
      first = false;
      ostr << *val;
    }
    return *this;
  }

  template <typename T>
  inline array& operator+=(T const& iVal)
  {
    if (!first)
      ostr << ", ";
    first               = false;
    json_ostream::operator=(iVal);
    return *this;
  }

  json_ostream next()
  {
    if (!first)
      ostr << ", ";
    first = false;
    return json_ostream(ostr);
  }

  template <typename Class>
  void print(Class const& obj)
  {
    for(auto const& each : obj)
    {
      using value_type = std::decay_t<decltype(each)>;
      if (!first)
        ostr << ", ";
      first = false;
      if constexpr (tuple_size<value_type> > 0)
        json_ostream::object(ostr).print(each);
      else if constexpr (std::is_same_v<value_type, bool>)
        ostr << std::boolalpha << static_cast<bool>(each);
      else if constexpr (std::is_same_v<value_type, float> ||
                         std::is_same_v<value_type, double>)
        ostr << static_cast<double>(each);
      else if constexpr (std::is_signed_v<value_type> &&
                         std::is_convertible_v<value_type, std::int64_t>)
        ostr << static_cast<std::int64_t>(each);
      else if constexpr (std::is_unsigned_v<value_type> &&
                         std::is_convertible_v<value_type, std::uint64_t>)
        ostr << static_cast<std::uint64_t>(each);
      else
        ostr << "\"" << std::string_view(each) << "\"";

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
  if constexpr (detail::is_same_decl_v<detail::decl_bound_obj, Decl>)
    json_ostream::object(ostr).print(decl.value(obj));
  else if constexpr (detail::is_same_decl_v<detail::decl_array_of_bound_obj, Decl> ||
      detail::is_same_decl_v<detail::decl_array_of_values, Decl>)
    json_ostream::array(ostr).print(decl.value(obj));
  else
  {
    if constexpr (std::is_same_v<typename Decl::ValueTag, string_tag>)
      ostr << "\"" << decl.value(obj) << "\"";
    else if constexpr (std::is_same_v<typename Decl::ValueTag, bool_tag>)
      ostr << std::boolalpha << decl.value(obj);
    else
      ostr << decl.value(obj);
  }

}


}