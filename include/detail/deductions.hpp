//
// Created by obhi on 9/18/20.
//
#pragma once

#include <concepts>
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

template <typename T>
auto from_string(std::string_view t)
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
concept BoundClass = tuple_size<Class> > 0;

template <typename Class, typename M>
concept BoundMember = BoundClass<M>;

// Strings
template <typename T>
concept StringType =
    std::is_same_v<std::string, T> || std::is_same_v<std::string_view, T> ||
    std::is_same_v<std::string, T> || std::is_same_v<char*, T> ||
    std::is_same_v<char const*, T>;

template <typename T>
concept CastableToStringView = requires(T t)
{
  std::string_view(t);
};

template <typename T>
concept CastableFromStringView = requires
{
  T(std::string_view());
};

template <typename T>
concept CastableFromString = requires
{
  T(std::string());
};

template <typename T>
concept TransformFromString = requires(T t)
{
  {
    jsb::from_string(std::string_view())
  }
  ->std::same_as<T>;
};

template <typename T>
concept ConvertibleToString = requires(T t)
{
  {
    std::to_string(t)
  }
  ->std::same_as<std::string>;
};

template <typename T>
concept TransformToString = requires(T t)
{
  {
    jsb::to_string(t)
  }
  ->std::same_as<std::string>;
};

template <typename T>
concept IsString = StringType<T> || CastableToStringView<T> ||
                   ConvertibleToString<T> || TransformToString<T>;

// Signed
template <typename T>
concept IsSigned = (std::IsSigned<T> && std::is_integral_v<T>) ||
                   (std::is_enum_v<T> &&
                    std::IsSigned<std::underlying_type_t<T>>);

// Unsigned
template <typename T>
concept IsUnsigned = (std::IsUnsigned<T> && std::is_integral_v<T>) ||
                     (std::is_enum_v<T> &&
                      std::IsUnsigned<std::underlying_type_t<T>>);

// Float
template <typename T>
concept IsFloat = std::is_floating_point_v<T>;

// Bool
template <typename T>
concept IsBool = std::is_same_v<T, bool>;

template <typename T>
concept IsSimpleValue =
    IsBool<T> || IsSigned<T> || IsUnsigned<T> || IsFloat<T> || IsString<T>;

// Array
template <typename Class>
concept Itereable = requires(Class obj)
{
  (*std::begin(obj));
  (*std::end(obj));
};

template <typename Class>
using array_value_type = std::decay_t<decltype(*std::begin(Class()))>;

template <typename Class>
concept ArrayOfObjects = Itereable<Class>&& BoundClass<array_value_type<Class>>;

template <typename Class>
concept ArrayOfValues = Itereable<Class> && !IsString<Class> &&
                        IsSimpleValue<array_value_type<Class>>;

template <typename Class>
concept IsArray = ArrayOfObjects<Class> || ArrayOfValues<Class>;

// Map
template <typename Class>
concept ValuePairList = requires(Class obj)
{
  (*std::begin(obj)).first;
  (*std::begin(obj)).second;
  (*std::end(obj)).first;
  (*std::end(obj)).second;
};

template <typename Class>
concept NamePairList = ValuePairList<Class>&&
    IsString<std::decay_t<decltype((*std::begin(Class())).first)>>;

template <typename Class>
concept IsMap = NamePairList<Class> || BoundClass<Class>;
} // namespace detail
} // namespace jsb