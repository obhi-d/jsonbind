//
// Created by obhi on 9/18/20.
//
#pragma once

#include <json_bind.hpp>

namespace jsb
{
template <typename> struct json_value_wrapper;

// returns an object if this value represents an object, or a null json_value
// template <typename V, typename R> R const& object(V const&);
// returns an object if this value represents an object, or a null json_value
// template <typename V, typename R> R const& array(V const&);
// returns true if json_value_type is valid
// template <typename V> bool valid(V const &);
// Iterator
// template <typename V, typename R> void map_for_each(V const&, R&&);
// Iterator
// template <typename V, typename R> void array_for_each(V const&, R&&);
// Float
// template <typename V> double as_float(V const&);
// Signed
// template <typename V> std::int64_t as_signed(V const&);
// Unsigned
// template <typename V> std::uint64_t as_unsigned(V const&);
// Bool
// Unsigned
// template <typename V> bool as_bool(V const&);
// String
// template <typename V> std::string_view as_string(V const&);
// Key, return a JsonValue const& object given a key and a map
// template <typename V, typename R> R const& key(std::string_view key, V const& map);

// json_value from a given API is transformed into
// a bound class using this interface
template <typename JsonValue>
class json_vstream
{
public:
  using json_value = JsonValue;
  using jv = json_value_wrapper<json_value>;
  class array;
  class object;

  array  as_array();
  object as_object();

  template <typename Value>
  inline void stream(Value& obj);

  json_vstream(json_vstream const&) noexcept = delete;
  explicit json_vstream(json_value const& i_val) noexcept : value(i_val) {}
  json_vstream(json_vstream&& i_other) noexcept : value(i_other.value) {}

protected:
  json_value const& value;
};

template <typename JsonValue>
class json_vstream<JsonValue>::object : public json_vstream<JsonValue>
{
public:
  using json_vstream<JsonValue>::json_value;

  explicit object(json_value const& ostr) : json_vstream<JsonValue>(ostr) {}

  object(object&& i_other) noexcept
      : json_vstream<JsonValue>(std::move(i_other))
  {
  }

  object(object const& ostr) noexcept = delete;
  ~object()                           = default;

  template <typename Class>
  requires(detail::BoundClass<Class>) void stream(Class& obj)
  {
    detail::set_all<Class>(*this, obj);
  }

  template <typename Class>
  requires(detail::NamePairList<Class>) void stream(Class& obj)
  {
    auto const& jv = jv::object(value);
    if (!jv::valid(jv))
    {
      return;
    }

    using nvname_t  = detail::nvname_t<Class>;
    using nvvalue_t = detail::nvvalue_t<Class>;

    detail::reserve(obj, detail::size(jv));

    jv::map_for_each(jv, [&obj](std::string_view key, JsonValue const& value) {
      nvvalue_t stream_val;
      json_vstream<JsonValue>(value).stream(stream_val);
      if constexpr (detail::IsString<nvname_t> ||
                    detail::CastableFromStringView<nvname_t>)
        detail::emplace(obj, nvname_t(key), std::move(stream_val));
      else if constexpr (detail::CastableFromString<nvname_t>)
        detail::emplace(obj, std::string(key), std::move(stream_val));
      else if constexpr (detail::TransformFromString<nvname_t>)
        detail::emplace(obj, jsb::from_string<nvname_t>(key),
                        std::move(stream_val));
    });
  }

  template <typename Class, typename Decl>
  inline void operator()(Class& obj, Decl const& decl);
};

template <typename JsonValue>
class json_vstream<JsonValue>::array : public json_vstream<JsonValue>
{
public:
  using json_vstream<JsonValue>::json_value;

  array(array&& i_other) noexcept : json_vstream<JsonValue>(std::move(i_other))
  {
    i_other.moved = true;
  }
  array(array const& ostr) noexcept = delete;
  explicit array(json_value const& value) : json_vstream<JsonValue>(value) {}
  ~array() {}

  template <typename Class>
  void stream(Class& obj)
  {
    auto const& jv = jv::array(value);
    if (!jv::valid(jv))
    {
      return;
    }

    detail::reserve(obj, detail::size(jv));
    jv::array_for_each(jv, [&obj](JsonValue const& value) {
      detail::array_value_t<Class> stream_val;
      json_vstream<JsonValue>(value).stream(stream_val);
      detail::emplace(obj, std::move(stream_val));
    });
  }

private:
};

template <typename JsonValue>
typename json_vstream<JsonValue>::array json_vstream<JsonValue>::as_array()
{
  return json_vstream<JsonValue>::array(value);
}

template <typename JsonValue>
typename json_vstream<JsonValue>::object json_vstream<JsonValue>::as_object()
{
  return json_vstream<JsonValue>::object(value);
}

template <typename JsonValue>
template <typename Value>
void json_vstream<JsonValue>::stream(Value& obj)
{
  using value_type = std::decay_t<Value>;

  if constexpr (detail::IsMap<value_type>)
    json_vstream<JsonValue>::object(value).stream(obj);
  else if constexpr (detail::IsArray<value_type>)
    json_vstream<JsonValue>::array(value).stream(obj);
  else if constexpr (detail::IsFloat<value_type>)
    obj = static_cast<value_type>(jv::as_float(value));
  else if constexpr (detail::IsSigned<value_type>)
    obj = static_cast<value_type>(jv::as_signed(value));
  else if constexpr (detail::IsUnsigned<value_type>)
    obj = static_cast<value_type>(jv::as_unsigned(value));
  else if constexpr (detail::IsBool<value_type>)
    obj = static_cast<value_type>(jv::as_bool(value));
  else if constexpr (detail::IsString<value_type>)
    obj = value_type(jv::as_string(value));
}

template <typename JsonValue, typename Class>
void stream_in(JsonValue const& jvalue, Class& obj)
{
  json_vstream<JsonValue>(jvalue).template stream<Class>(obj);
}

template <typename JsonValue>
template <typename Class, typename Decl>
void json_vstream<JsonValue>::object::operator()(Class& obj, Decl const& decl)
{
  auto const& key_val = jv::key(decl.key(), value);
  if (jv::valid(key_val))
  {
    using value_t = typename Decl::MemTy;
    value_t load;
    json_vstream<JsonValue>(key_val).stream(load);
    decl.value(obj, std::move(load));
  }
}

} // namespace jsb