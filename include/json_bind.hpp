/*
 * JsonBind.h
 *
 *  Created on: 17 Sep 2020, 15:59:40
 *      Author: obhi
 */
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace jsb
{

// @remarks
// Highly borrowed from
// https://github.com/eliasdaler/MetaStuff

struct bool_tag
{
  using type = bool;
};
struct float_tag
{
  using type = double;
};
struct unsigned_tag
{
  using type = std::uint64_t;
};
struct signed_tag
{
  using type = std::int64_t;
};
struct string_tag
{
  using type = std::string_view;
};
struct object_tag
{
  using type = std::string;
};
struct array_tag
{
  using type = std::string;
};

static inline constexpr bool_tag     bool_;
static inline constexpr float_tag    float_;
static inline constexpr unsigned_tag unsigned_;
static inline constexpr signed_tag   signed_;
static inline constexpr string_tag   string_;
static inline constexpr object_tag   object_;
static inline constexpr array_tag    array_;

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
template <typename T>
inline constexpr bool is_string_v = std::is_same_v<std::string, T> ||
    std::is_same_v<std::string_view, T> ||
    std::is_same_v<std::string, T> ||
    std::is_same_v<char*, T> ||
    std::is_same_v<char const*, T>;

template <typename Class = void>
auto decl()
{
  return std::tuple<>();
}

template <typename Class, typename M>
using array_val_t = std::decay_t<decltype(*std::begin(M()))>;
template <typename Class>
using decl_t = std::decay_t<decltype(decl<Class>())>;
template <typename Class>
inline constexpr std::size_t tuple_size = std::tuple_size_v<decl_t<Class>>;

template <typename Class, typename M>
concept is_bound_obj_list =
    std::tuple_size_v<decl_t<array_val_t<Class, M>>> > 0;

template <typename Class, typename M>
concept is_value_list =
    std::is_convertible_v<array_val_t<Class, M>, bool> ||
    std::is_convertible_v<array_val_t<Class, M>, unsigned int> ||
    std::is_convertible_v<array_val_t<Class, M>, int> ||
    std::is_convertible_v<array_val_t<Class, M>, float> ||
    std::is_convertible_v<array_val_t<Class, M>, std::string>;

template <typename Class, typename M>
concept is_member_bound = tuple_size<M> > 0;

template <typename Class>
concept is_named_map_type = requires(Class obj)
{
  (*std::begin(obj)).first;
  std::string_view((*std::begin(obj)).first);
  (*std::begin(obj)).second;
  (*std::end(obj)).first;
  (*std::end(obj)).second;
};
namespace detail
{

template <typename Class, typename M, typename Tag>
class decl_base
{
public:
  using ClassTy  = Class;
  using ValueTag = Tag;
  using MemTy    = M;

  constexpr decl_base(std::string_view iName) : name(iName) {}

  std::string_view key() const
  {
    return name;
  }

private:
  std::string_view name = nullptr;
};

template <typename Class, typename M, typename Tag>
class decl_member_ptr : public decl_base<Class, M, Tag>
{
public:
  using ValueTy = typename Tag::type;

  constexpr decl_member_ptr(std::string_view iName, member_ptr<Class, M> iPtr)
      : decl_base<Class, M, Tag>(iName), member(iPtr)
  {
  }

  void value(Class& obj, ValueTy value) const
  {
    obj.*member = static_cast<M>(value);
  }

  ValueTy value(Class const& obj) const
  {
    return static_cast<ValueTy>(obj.*member);
  }

protected:
  member_ptr<Class, M> member = nullptr;
};

template <typename Class, typename M, typename Tag>
class decl_get_set : public decl_base<Class, M, Tag>
{
public:
  using ValueTy = typename Tag::type;

  constexpr decl_get_set(std::string_view iName, get_fn<Class, ValueTy> iGetter,
                         set_fn<Class, ValueTy> iSetter)
      : decl_base<Class, M, Tag>(iName), getter(iGetter), setter(iSetter)
  {
  }

  void value(Class& obj, ValueTy value) const
  {
    (obj.*setter)(value);
  }

  ValueTy value(Class const& obj) const
  {
    return static_cast<ValueTy>((obj.*getter)());
  }

protected:
  get_fn<Class, ValueTy> getter = nullptr;
  set_fn<Class, ValueTy> setter = nullptr;
};

template <typename Class, typename M, typename Tag>
class decl_free_get_set : public decl_base<Class, M, Tag>
{
public:
  using ValueTy = typename Tag::type;

  constexpr decl_free_get_set(std::string_view            iName,
                              free_get_fn<Class, ValueTy> iGetter,
                              free_set_fn<Class, ValueTy> iSetter)
      : decl_base<Class, M, Tag>(iName), free_getter(iGetter),
        free_setter(iSetter)
  {
  }

  void value(Class& obj, ValueTy value) const
  {
    free_setter(obj, value);
  }

  ValueTy value(Class const& obj) const
  {
    return free_getter(obj);
  }

protected:
  free_get_fn<Class, ValueTy> free_getter = nullptr;
  free_set_fn<Class, ValueTy> free_setter = nullptr;
};

template <typename Class, typename M, typename Tag>
class decl_bound_obj : public decl_member_ptr<Class, M, Tag>
{
public:
  constexpr decl_bound_obj(std::string_view iName, member_ptr<Class, M> iPtr)
      : decl_member_ptr<Class, M, Tag>(iName, iPtr)
  {
  }

  M const& value(Class const& obj) const
  {
    return (obj.*decl_member_ptr<Class, M, Tag>::member);
  }

private:
};

template <typename Class, typename M, typename Tag>
class decl_array_of_bound_obj : public decl_member_ptr<Class, M, Tag>
{
public:
  constexpr decl_array_of_bound_obj(std::string_view     iName,
                                    member_ptr<Class, M> iPtr)
      : decl_member_ptr<Class, M, Tag>(iName, iPtr)
  {
  }

  M const& value(Class const& obj) const
  {
    return (obj.*decl_member_ptr<Class, M, Tag>::member);
  }
};

template <typename Class, typename M, typename Tag>
class decl_array_of_values : public decl_member_ptr<Class, M, Tag>
{
public:
  constexpr decl_array_of_values(std::string_view     iName,
                                 member_ptr<Class, M> iPtr)
      : decl_member_ptr<Class, M, Tag>(iName, iPtr)
  {
  }

  M const& value(Class const& obj) const
  {
    return (obj.*decl_member_ptr<Class, M, Tag>::member);
  }
};

template <template <typename Class, typename M, typename Tag> class Decl,
          class ArgDecl>
inline constexpr bool is_same_decl_v =
    std::is_same_v<Decl<typename ArgDecl::ClassTy, typename ArgDecl::MemTy,
                        typename ArgDecl::ValueTag>,
                   ArgDecl>;

template <typename Class>
auto const& get_decl()
{
  static const auto json_bind_ = decl<Class>();
  return json_bind_;
}

template <typename TupleTy, typename Class, typename Fn, size_t... I>
void apply(Fn&& fn, Class& obj, TupleTy&& tup, std::index_sequence<I...>)
{
  (fn(obj, std::get<I>(tup)), ...);
}

template <typename TupleTy, typename Class, typename Fn, size_t... I>
void apply(Fn&& fn, Class const& obj, TupleTy&& tup, std::index_sequence<I...>)
{
  (fn(obj, std::get<I>(tup)), ...);
}

template <typename Class, typename Fn>
void for_all(Fn&& fn, Class& obj)
{
  apply(std::forward<Fn>(fn), obj, get_decl<Class>(),
        std::make_index_sequence<tuple_size<Class>>());
}

template <typename Class, typename Fn>
void for_all(Fn&& fn, Class const& obj)
{
  apply(std::forward<Fn>(fn), obj, get_decl<Class>(),
        std::make_index_sequence<tuple_size<Class>>());
}



} // namespace detail

template <typename Class, typename M, typename Tag>
requires((std::is_convertible_v<typename Tag::type, M> && std::is_convertible_v<
         M, typename Tag::type>) || is_string_v<M>) constexpr auto bind(std::string_view     iName,
                                                     member_ptr<Class, M> iPtr,
                                                     Tag)
{
  return detail::decl_member_ptr<Class, M, Tag>(iName, iPtr);
}

template <typename Class, typename M>
requires(is_member_bound<Class, M>) constexpr auto bind(
    std::string_view iName, member_ptr<Class, M> iPtr, object_tag)
{
  return detail::decl_bound_obj<Class, M, object_tag>(iName, iPtr);
}

template <typename Class, typename M>
requires(is_bound_obj_list<Class, M>) constexpr auto bind(
    std::string_view iName, member_ptr<Class, M> iPtr, array_tag)
{
  return detail::decl_array_of_bound_obj<Class, M, array_tag>(iName, iPtr);
}

template <typename Class, typename M>
requires(is_value_list<Class, M>) constexpr auto bind(std::string_view iName,
                                                      member_ptr<Class, M> iPtr,
                                                      array_tag)
{
  return detail::decl_array_of_values<Class, M, array_tag>(iName, iPtr);
}

template <typename Class, typename M, typename Tag>
constexpr auto bind(std::string_view                  iName,
                    get_fn<Class, typename Tag::type> iGetter,
                    set_fn<Class, typename Tag::type> iSetter, Tag)
{
  return detail::decl_get_set<Class, M, Tag>(iName, iGetter, iSetter);
}

template <typename Class, typename M, typename Tag>
constexpr auto bind(std::string_view                       iName,
                    free_get_fn<Class, typename Tag::type> iGetter,
                    free_set_fn<Class, typename Tag::type> iSetter, Tag)
{
  return detail::decl_free_get_set<Class, M, Tag>(iName, iGetter, iSetter);
}

template <typename... Args>
auto json_bind(Args&&... args)
{
  return std::make_tuple(std::forward<Args>(args)...);
}

} // namespace jsb