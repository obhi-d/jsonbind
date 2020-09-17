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
    assert(type == Tag::eValue);
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
    begin();
  }
  object(object const& ostr) noexcept = delete;
  ~object() { end(); }

  template <typename Class>
  void print(Class const& obj)
  {
    for_all<Class>(*this);
  }

  template <typename Decl>
  inline void operator() (Decl const& decl)
  {

  }

private:
  void begin()
  {
    ostr << "{ ";
  }
  void end()
  {
    ostr << " }";
  }
  bool first = true;
};

class json_ostream::array : public json_ostream
{
public:
  array(array&& i_other) noexcept
      : json_ostream(std::move(i_other)), first(i_other.first)
  {
  }
  array(array const& ostr) noexcept = delete;
  explicit array(std::ostream& ostr) : json_ostream(ostr) {}

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

private:
  void begin()
  {
    ostr << "[ ";
  }
  void end()
  {
    ostr << " ]";
  }
  bool first = true;
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


}