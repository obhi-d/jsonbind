//
// Created by obhi on 9/18/20.
//

#pragma once
#include <detail/deductions.hpp>

namespace jsb::detail
{
template <typename T>
requires(detail::StringType<T> ||
         detail::CastableToStringView<T>) static std::string_view
    as_string(T const& val)
{
  return std::string_view(val);
}

template <typename T>
requires(!detail::StringType<T> && !detail::CastableToStringView<T> &&
         !detail::TransformToString<T> &&
         detail::ConvertibleToString<T>) static std::string
    as_string(T const& val)
{
  return std::to_string(val);
}

template <typename T>
requires(!detail::StringType<T> && !detail::CastableToStringView<T> &&
         detail::TransformToString<T>) static std::string
    as_string(T const& val)
{
  return jsb::to_string(val);
}
} // namespace jsb::detail