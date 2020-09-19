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
template <auto>
struct member_ptr_t;

template <typename T, typename M, M T::*P>
struct member_ptr_t<P>
{
  using class_t  = std::decay_t<T>;
  using member_t = std::decay_t<M>;
};

template <auto>
struct member_getter_t;

template <typename T, typename R, R (T::*MF)() const>
struct member_getter_t<MF>
{
  using class_t  = std::decay_t<T>;
  using return_t = R;
  using value_t  = std::decay_t<R>;
};

template <auto>
struct member_setter_t;

template <typename T, typename R, void (T::*MF)(R)>
struct member_setter_t<MF>
{
  using class_t  = std::decay_t<T>;
  using return_t = R;
  using value_t  = std::decay_t<R>;
};

template <auto>
struct free_getter_t;

template <typename T, typename R, R (*F)(T const&)>
struct free_getter_t<F>
{
  using class_t  = std::decay_t<T>;
  using return_t = R;
  using value_t  = std::decay_t<R>;
};

template <auto>
struct free_setter_t;

template <typename T, typename R, void (*F)(T&, R)>
struct free_setter_t<F>
{
  using class_t  = std::decay_t<T>;
  using return_t = R;
  using value_t  = std::decay_t<R>;
};

// Concepts
// Decl
template <typename Class>
using decl_t = std::decay_t<decltype(decl<std::decay_t<Class>>())>;

// Utils
template <typename Class>
inline constexpr std::size_t tuple_size =
    std::tuple_size_v<decl_t<std::decay_t<Class>>>;

template <typename Class>
concept BoundClass = tuple_size<std::decay_t<Class>> > 0;

template <auto MPtr>
concept IsMemberPtr = requires
{
  typename member_ptr_t<MPtr>::class_t;
  typename member_ptr_t<MPtr>::member_t;
};

template <auto Getter, auto Setter>
concept IsMemberGetterSetter = requires
{
  typename member_getter_t<Getter>::return_t;
  typename member_getter_t<Getter>::class_t;
  typename member_setter_t<Setter>::return_t;
  typename member_setter_t<Setter>::class_t;
};

template <auto Getter, auto Setter>
concept IsFreeGetterSetter = requires
{
  typename free_getter_t<Getter>::return_t;
  typename free_getter_t<Getter>::class_t;
  typename free_setter_t<Setter>::return_t;
  typename free_setter_t<Setter>::class_t;
};

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
concept TransformFromString = requires
{
  {
    jsb::from_string<T>(std::string_view())
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
concept IsSigned = (std::is_signed_v<T> && std::is_integral_v<T>) ||
                   (std::is_enum_v<T> &&
                    std::is_signed_v<std::underlying_type_t<T>>);

// Unsigned
template <typename T>
concept IsUnsigned = (std::is_unsigned_v<T> && std::is_integral_v<T>) ||
                     (std::is_enum_v<T> &&
                      std::is_unsigned_v<std::underlying_type_t<T>>);

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
using array_value_t = std::decay_t<decltype(*std::begin(Class()))>;

template <typename Class>
concept ArrayOfObjects = Itereable<Class>&& BoundClass<array_value_t<Class>>;

template <typename Class>
concept ArrayOfValues =
    Itereable<Class> && !IsString<Class> && IsSimpleValue<array_value_t<Class>>;

template <typename Class>
concept IsArray = ArrayOfObjects<Class> || ArrayOfValues<Class>;

template <typename Class>
concept HasValueType = requires(Class obj)
{
  typename Class::value_type;
};

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
concept HasReserve = requires(std::decay_t<Class> obj)
{
  obj.reserve(std::size_t());
};

template <typename Class>
concept HasSize = requires(Class obj)
{
  {
    obj.size()
  }
  ->std::convertible_to<std::size_t>;
};

template <typename Class, typename ValueType>
concept HasEmplace = requires(Class obj, ValueType value)
{
  obj.emplace(value);
};

template <typename Class, typename ValueType>
concept HasPushBack = requires(Class obj, ValueType value)
{
  obj.push_back(value);
};

template <typename Class, typename ValueType>
concept HasEmplaceBack = requires(Class obj, ValueType value)
{
  obj.emplace_back(value);
};

template <typename Class>
concept NamePairList = ValuePairList<Class>&&
    IsString<std::decay_t<decltype((*std::begin(Class())).first)>>;

template <typename Class>
requires(HasValueType<Class>) using container_value_t =
    typename Class::value_type;

template <typename Class>
requires(NamePairList<Class>) using nvname_t =
    std::decay_t<decltype((*std::begin(Class())).first)>;
template <typename Class>
requires(NamePairList<Class>) using nvvalue_t =
    std::decay_t<decltype((*std::begin(Class())).second)>;

template <typename Class>
concept IsMap = NamePairList<Class> || BoundClass<Class>;
} // namespace detail
} // namespace jsb