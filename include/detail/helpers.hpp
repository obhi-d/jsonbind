//
// Created by obhi on 9/18/20.
//

#pragma once
#include <detail/deductions.hpp>

namespace jsb::detail
{
template <typename T>
requires(detail::IsBasicString<T> ||
         detail::CastableToStringView<T>) static inline std::string_view
    as_string(T const& val)
{
  return std::string_view(val);
}

template <typename T>
requires(!detail::IsBasicString<T> && !detail::CastableToStringView<T> &&
         !detail::TransformToString<T> &&
         detail::ConvertibleToString<T>) static inline std::string
    as_string(T const& val)
{
  return std::to_string(val);
}

template <typename T>
requires(
    !detail::IsBasicString<T> && !detail::CastableToStringView<T> &&
    (detail::TransformToString<T> ||
     detail::TransformToStringView<T>)) static inline auto as_string(T const&
                                                                         val)
{
  return jsb::string_transform<T>::to_string(val);
}

template <typename C, typename... Args>
requires(
    detail::HasValueType<C>&& detail::HasEmplace<C, container_value_t<C>> &&
    !detail::HasEmplaceBack<
        C, container_value_t<C>>) static inline void emplace(C& c,
                                                             Args&&... args)
{
  c.emplace(std::forward<Args>(args)...);
}

template <typename C, typename... Args>
requires(
    detail::HasValueType<C> && !detail::HasEmplace<C, container_value_t<C>> &&
    detail::HasEmplaceBack<
        C, container_value_t<C>>) static inline void emplace(C& c,
                                                             Args&&... args)
{
  c.emplace_back(std::forward<Args>(args)...);
}

template <typename C, typename... Args>
requires(
    detail::HasValueType<C> && !detail::HasEmplace<C, container_value_t<C>> &&
    !detail::HasEmplaceBack<C, container_value_t<C>> &&
    detail::HasPushBack<
        C, container_value_t<C>>) static inline void emplace(C& c,
                                                             Args&&... args)
{
  c.push_back(std::forward<Args>(args)...);
}

template <typename C>
requires(!detail::HasReserve<C>) static inline void reserve(C&          c,
                                                            std::size_t sz)
{
}

template <typename C>
requires(detail::HasReserve<C>) static inline void reserve(C& c, std::size_t sz)
{
  c.reserve(sz);
}

template <typename C>
requires(!detail::HasSize<C>) static inline std::size_t size(C const& c)
{
  return std::size_t();
}

template <typename C>
requires(detail::HasSize<C>) static inline std::size_t size(C const& c)
{
  return static_cast<std::size_t>(c.size());
}

} // namespace jsb::detail