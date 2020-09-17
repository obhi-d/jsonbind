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

static inline constexpr bool_tag     t_bool;
static inline constexpr float_tag    t_float;
static inline constexpr unsigned_tag t_unsigned;
static inline constexpr signed_tag   t_signed;
static inline constexpr string_tag   t_string;
static inline constexpr object_tag   t_object;
static inline constexpr array_tag    t_array;

template <typename Class, typename M>
using member_ptr = M Class::*;

template <typename Class, typename M, typename ValType>
using get = ValType (Class::*)() const;
template <typename Class, typename M, typename ValType>
using free_get = ValType (*)(Class const&);
template <typename Class, typename M, typename ValType>
using set = void (Class::*)(ValType);
template <typename Class, typename M, typename ValType>
using free_set = void (*)(Class&, ValType);

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

template <typename Class = void>
auto decl()
{
  return std::tuple<>();
}

template <typename Class, typename M, typename Tag>
class decl_base
{
public:
  using ClassTy = Class;
  using TagTy = Tag;
  using MemTy = M;

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

private:
  member_ptr<Class, M> member = nullptr;
};

template <typename Class, typename M, typename Tag>
class decl_get_set : public decl_base<Class, M, Tag>
{
public:
  using ValueTy = typename Tag::type;

  constexpr decl_get_set(std::string_view iName, get<Class, M, ValueTy> iGetter,
                         set<Class, M, ValueTy> iSetter)
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

private:
  get<Class, M, ValueTy> getter = nullptr;
  set<Class, M, ValueTy> setter = nullptr;
};

template <typename Class, typename M, typename Tag>
class decl_free_get_set : public decl_base<Class, M, Tag>
{
public:
  using ValueTy = typename Tag::type;

  constexpr decl_free_get_set(std::string_view            iName,
                              free_get<Class, M, ValueTy> iGetter,
                              free_set<Class, M, ValueTy> iSetter)
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

private:
  free_get<Class, M, ValueTy> free_getter = nullptr;
  free_set<Class, M, ValueTy> free_setter = nullptr;
};

template <typename Class, typename M, typename Tag>
class decl_bound_obj : public decl_member_ptr<Class, M, Tag>
{
public:
  constexpr decl_bound_obj(std::string_view iName, member_ptr<Class, M> iPtr)
      : decl_member_ptr<Class, M, Tag>(iName, iPtr)
  {
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
};

template <typename Class>
static const auto json_bind_ = decl<Class>();

template <typename Class>
auto const& get_decl()
{
  return json_bind_<Class>;
}

template <typename Class, typename M, typename Tag>
requires(std::is_convertible_v<typename Tag::type, M>&& std::is_convertible_v<
         M, typename Tag::type>) constexpr auto bind(std::string_view     iName,
                                                     member_ptr<Class, M> iPtr,
                                                     Tag)
{
  return decl_member_ptr<Class, M, Tag>(iName, iPtr);
}

template <typename Class, typename M>
requires(is_member_bound<Class, M>) constexpr auto bind(
    std::string_view iName, member_ptr<Class, M> iPtr, object_tag)
{
  return decl_bound_obj<Class, M, object_tag>(iName, iPtr);
}

template <typename Class, typename M>
requires(is_bound_obj_list<Class, M>) constexpr auto bind(
    std::string_view iName, member_ptr<Class, M> iPtr, array_tag)
{
  return decl_array_of_bound_obj<Class, M, array_tag>(iName, iPtr);
}

template <typename Class, typename M>
requires(is_value_list<Class, M>) constexpr auto bind(std::string_view iName,
                                                      member_ptr<Class, M> iPtr,
                                                      array_tag)
{
  return decl_array_of_values<Class, M, array_tag>(iName, iPtr);
}

template <typename Class, typename M, typename Tag>
constexpr auto bind(std::string_view                  iName,
                    get<Class, M, typename Tag::type> iGetter,
                    set<Class, M, typename Tag::type> iSetter, Tag)
{
  return decl_get_set<Class, M, Tag>(iName, iGetter, iSetter);
}

template <typename Class, typename M, typename Tag>
constexpr auto bind(std::string_view                       iName,
                    free_get<Class, M, typename Tag::type> iGetter,
                    free_set<Class, M, typename Tag::type> iSetter, Tag)
{
  return decl_free_get_set<Class, M, Tag>(iName, iGetter, iSetter);
}

template <typename... Args>
auto json_bind(Args&&... args)
{
  return std::make_tuple(std::forward<Args>(args)...);
}

template <typename TupleTy, typename Fn, size_t... I>
void apply(Fn&& fn, TupleTy&& tup, std::index_sequence<I...>)
{
  (fn(std::get<I>(tup)), ...);
}

template <typename Class, typename Fn>
void for_all(Fn&& fn)
{
  apply(std::forward<Fn>(fn), get_decl<Class>(),
        std::make_index_sequence<tuple_size<Class>>());
}
} // namespace jsb