//
// Created by obhi on 9/18/20.
//
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace jsb
{
template <typename T>
auto to_string(T const&)
{
}

template <typename Class = void>
auto decl()
{
  return std::tuple<>();
}

namespace detail
{

// Types
template <typename Class, typename M>
using member_ptr = M Class::*;
template <typename Class, typename ValType>
using get_fn = ValType (Class::*)() const;
template <typename Class, typename ValType>
using free_get_fn = ValType (*)(Class const&);
template <typename Class, typename ValType>
using set_fn = void (Class::*)(ValType);
template <typename Class, typename ValType>
using free_set_fn = void (*)(Class&, ValType);

// Concepts
// Decl
template <typename Class>
using decl_t = std::decay_t<decltype(decl<Class>())>;

// Utils
template <typename Class>
inline constexpr std::size_t tuple_size = std::tuple_size_v<decl_t<Class>>;

template <typename Class>
concept is_class_bound = tuple_size<Class> > 0;

template <typename Class, typename M>
concept is_member_bound = is_class_bound<M>;

// Strings
template <typename T>
concept is_string_type =
    std::is_same_v<std::string, T> || std::is_same_v<std::string_view, T> ||
    std::is_same_v<std::string, T> || std::is_same_v<char*, T> ||
    std::is_same_v<char const*, T>;

template <typename T>
concept is_string_ctor_valid = requires(T t)
{
  std::string_view(t);
};

template <typename T>
concept is_string_convertible = requires(T t)
{
  {
    std::to_string(t)
  }
  ->std::same_as<std::string>;
};

template <typename T>
concept has_string_transform = requires(T t)
{
  {
    jsb::to_string(t)
  }
  ->std::same_as<std::string>;
};

template <typename T>
concept is_string_v = is_string_type<T> || is_string_ctor_valid<T> ||
                      is_string_convertible<T> || has_string_transform<T>;

// Signed
template <typename T>
concept is_signed_v = (std::is_signed_v<T> && std::is_integral_v<T>) ||
                      (std::is_enum_v<T> &&
                       std::is_signed_v<std::underlying_type_t<T>>);

// Unsigned
template <typename T>
concept is_unsigned_v = (std::is_unsigned_v<T> && std::is_integral_v<T>) ||
                        (std::is_enum_v<T> &&
                         std::is_unsigned_v<std::underlying_type_t<T>>);

// Float
template <typename T>
concept is_float_v = std::is_floating_point_v<T>;

// Bool
template <typename T>
concept is_bool_v = std::is_same_v<T, bool>;

template <typename T>
concept is_value = is_bool_v<T> || is_signed_v<T> || is_unsigned_v<T> ||
                   is_float_v<T> || is_string_v<T>;

// Array
template <typename Class>
concept is_iterable = requires(Class obj)
{
  (*std::begin(obj));
  (*std::end(obj));
};

template <typename Class>
using array_value_type = std::decay_t<decltype(*std::begin(Class()))>;

template <typename Class>
concept is_array_of_objects =
    is_iterable<Class>&& is_class_bound<array_value_type<Class>>;

template <typename Class>
concept is_array_of_values = is_iterable<Class> && !is_string_v<Class> &&
                             is_value<array_value_type<Class>>;

template <typename Class>
concept is_array_v = is_array_of_objects<Class> || is_array_of_values<Class>;

// Map
template <typename Class>
concept is_value_pair_list = requires(Class obj)
{
  (*std::begin(obj)).first;
  (*std::begin(obj)).second;
  (*std::end(obj)).first;
  (*std::end(obj)).second;
};

template <typename Class>
concept is_name_pair_list = is_value_pair_list<Class>&&
    is_string_v<std::decay_t<decltype((*std::begin(Class())).first)>>;

template <typename Class>
concept is_map_v = is_name_pair_list<Class> || is_class_bound<Class>;

// Deduced decl type check
template <template <typename Class, typename M> class Decl, class ArgDecl>
inline constexpr bool is_same_decl_v =
    std::is_same_v<Decl<typename ArgDecl::ClassTy, typename ArgDecl::MemTy>,
                   ArgDecl>;

} // namespace detail
} // namespace jsb