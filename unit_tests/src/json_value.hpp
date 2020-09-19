//
// Created by obhi on 9/19/20.
//

#include <json_vstream.hpp>
#include <nlohmann/json.hpp>

namespace jsb
{
inline nlohmann::json const& jv_null()
{
  static nlohmann::json const null;
  return null;
}
// returns an object if this value represents an object, or a null json_value
inline auto const& jv_object(nlohmann::json const& val)
{
  if (!val.is_object())
    return jv_null();
  return val;
}
// returns an object if this value represents an object, or a null json_value
inline auto const& jv_array(nlohmann::json const& val)
{
  if (!val.is_array())
    return jv_null();
  return val;
}
// returns true if json_value_type is valid
inline bool jv_valid(nlohmann::json const& val)
{
  return !val.is_null();
}
// Iterator
template <typename Lambda>
inline void jv_map_for_each(nlohmann::json const& val, Lambda&& l)
{
  if (val.is_object())
  {
    for (auto it = val.begin(); it != val.end(); ++it)
    {
      auto const& kt = it.key();
      l(std::string_view(kt.c_str(), kt.length()), it.value());
    }
  }
}
template <typename Lambda>
inline void jv_array_for_each(nlohmann::json const& val, Lambda&& l)
{
  if (val.is_array())
  {
    for (auto it = val.begin(); it != val.end(); ++it)
    {
      nlohmann::json const& el = *it;
      l(el);
    }
  }
}
// Float
inline double jv_float(nlohmann::json const& val)
{
  if (val.is_number_float())
  {
    return val.get<double>();
  }
  if (val.is_number())
  {
    return static_cast<double>(val.get<std::int64_t>());
  }
  if (val.is_number_unsigned())
  {
    return static_cast<double>(val.get<std::uint64_t>());
  }
  return 0.0;
}
// Signed
inline std::int64_t jv_signed(nlohmann::json const& val)
{
  if (val.is_number())
  {
    return val.get<std::int64_t>();
  }
  if (val.is_number_unsigned())
  {
    return static_cast<std::int64_t>(val.get<std::uint64_t>());
  }
  return 0;
}
// Unsigned
inline std::uint64_t jv_unsigned(nlohmann::json const& val)
{
  if (val.is_number_unsigned())
  {
    return val.get<std::uint64_t>();
  }
  if (val.is_number())
  {
    return static_cast<std::uint64_t>(val.get<std::int64_t>());
  }
  return 0;
}
// Bool
inline bool jv_bool(nlohmann::json const& val)
{
  if (val.is_boolean())
    return val.get<bool>();
  return false;
}
// String
inline auto jv_string(nlohmann::json const& val)
{
  if (val.is_string())
  {
    return val.get<std::string_view>();
  }
  return std::string_view();
}
// Key, return a JsonValue const& object given a key and a map
inline auto jv_key(std::string_view key, nlohmann::json const& val)
{
  if (val.is_object())
  {
    auto it = val.find(key);
    if (it != val.end())
    {
      return *it;
    }
  }
  return jv_null();
}

} // namespace jsb