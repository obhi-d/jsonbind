//
// Created by obhi on 9/18/20.
//

#pragma once
#include <detail/deductions.hpp>

namespace jsb::detail
{

template <typename T>
requires(detail::is_string_type<T> ||
         detail::is_string_ctor_valid<T>) static std::string_view
    as_string(T const& val)
{
  return std::string_view(val);
}

template <typename T>
requires(!detail::is_string_type<T> && !detail::is_string_ctor_valid<T> &&
         !detail::has_string_transform<T> &&
         detail::is_string_convertible<T>) static std::string
    as_string(T const& val)
{
  return std::to_string(val);
}

template <typename T>
requires(!detail::is_string_type<T> && !detail::is_string_ctor_valid<T> &&
         detail::has_string_transform<T>) static std::string
    as_string(T const& val)
{
  return jsb::to_string(val);
}

}