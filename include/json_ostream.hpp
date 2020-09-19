//
// Created by obhi on 9/17/20.
//
#pragma once

#include <cassert>
#include <json_bind.hpp>
#include <ostream>
#include <string>
#include <string_view>

namespace jsb
{
class json_ostream
{
public:
  explicit json_ostream(std::ostream& i_ostr) noexcept : ostr(i_ostr) {}

  class array;
  class object;

  array  as_array();
  object as_object();

  template <typename Value>
  inline void stream(Value const& obj);

  json_ostream(json_ostream const&) noexcept = delete;

protected:
  json_ostream(json_ostream&& i_other) noexcept : ostr(i_other.ostr) {}

protected:
  std::ostream& ostr;
};

class json_ostream::object : public json_ostream
{
public:
  explicit object(std::ostream& ostr) : json_ostream(ostr)
  {
    begin();
  }
  object(object&& i_other) noexcept
      : json_ostream(std::move(i_other)), first(i_other.first)
  {
    i_other.moved = true;
  }

  object(object const& ostr) noexcept = delete;
  ~object()
  {
    end();
  }

  template <typename Class>
  requires(detail::BoundClass<Class>) void stream(Class const& obj)
  {
    detail::for_all<Class>(*this, obj);
  }

  template <typename Class>
  requires(detail::NamePairList<Class>) void stream(Class const& obj)
  {
    for (auto& pair : obj)
    {
      if (!first)
        ostr << ", ";
      first = false;
      ostr << "\"" << detail::as_string(pair.first) << "\": ";
      json_ostream::stream(pair.second);
    }
  }

  template <typename Class, typename Decl>
  inline void operator()(Class const& obj, Decl const& decl);

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
  explicit array(std::ostream& ostr) : json_ostream(ostr)
  {
    begin();
  }
  ~array()
  {
    end();
  }

  template <typename Class>
  void stream(Class const& obj)
  {
    for (auto const& each : obj)
    {
      using value_type = std::decay_t<decltype(each)>;
      if (!first)
        ostr << ", ";
      first = false;
      json_ostream::stream(each);
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
void json_ostream::stream(const Value& obj)
{
  using value_type = std::decay_t<Value>;

  if constexpr (detail::IsMap<value_type>)
    json_ostream::object(ostr).stream(obj);
  else if constexpr (detail::IsArray<value_type>)
    json_ostream::array(ostr).stream(obj);
  else if constexpr (detail::IsFloat<value_type>)
    ostr << static_cast<double>(obj);
  else if constexpr (detail::IsSigned<value_type>)
    ostr << static_cast<std::int64_t>(obj);
  else if constexpr (detail::IsUnsigned<value_type>)
    ostr << static_cast<std::uint64_t>(obj);
  else if constexpr (detail::IsBool<value_type>)
    ostr << std::boolalpha << static_cast<bool>(obj);
  else if constexpr (detail::IsString<value_type>)
    ostr << "\"" << detail::as_string(obj) << "\"";
}

template <typename Class>
void stream_out(std::ostream& ostr, Class const& obj)
{
  if constexpr (detail::BoundClass<Class>)
    json_ostream(ostr).stream(obj);
}

template <typename Class, typename Decl>
void json_ostream::object::operator()(Class const& obj, const Decl& decl)
{
  if (!first)
    ostr << ", ";
  first = false;

  ostr << "\"" << decl.key() << "\": ";
  json_ostream::stream(decl.value(obj));
}
} // namespace jsb