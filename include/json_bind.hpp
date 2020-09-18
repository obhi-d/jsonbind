/*
 * JsonBind.h
 *
 *  Created on: 17 Sep 2020, 15:59:40
 *      Author: obhi
 */
#pragma once
#include <detail/helpers.hpp>

namespace jsb
{

namespace detail
{

// @remarks
// Highly borrowed from
// https://github.com/eliasdaler/MetaStuff
template <typename Class, typename M>
class decl_base
{
public:
  using ClassTy  = Class;
  using MemTy    = M;

  constexpr decl_base(std::string_view iName) : name(iName) {}

  std::string_view key() const
  {
    return name;
  }

private:
  std::string_view name = nullptr;
};

template <typename Class, typename M>
class decl_member_ptr : public decl_base<Class, M>
{
public:

  constexpr decl_member_ptr(std::string_view iName, member_ptr<Class, M> iPtr)
      : decl_base<Class, M>(iName), member(iPtr)
  {
  }

  inline void value(Class& obj, M const& value) const
  {
    obj.*member = value;
  }

  inline void value(Class& obj, M&& value) const
  {
    obj.*member = std::move(value);
  }

  M const& value(Class const& obj) const
  {
    return (obj.*member);
  }

protected:
  member_ptr<Class, M> member = nullptr;
};

template <typename Class, typename M>
class decl_get_set : public decl_base<Class, M>
{
public:

  constexpr decl_get_set(std::string_view iName, get_fn<Class, M> iGetter,
                         set_fn<Class, M> iSetter)
      : decl_base<Class, M>(iName), getter(iGetter), setter(iSetter)
  {
  }

  inline void value(Class& obj, M const& value) const
  {
    (obj.*setter)(value);
  }

  M const& value(Class const& obj) const
  {
    return ((obj.*getter)());
  }

protected:
  get_fn<Class, M> getter = nullptr;
  set_fn<Class, M> setter = nullptr;
};

template <typename Class, typename M>
class decl_free_get_set : public decl_base<Class, M>
{
public:

  constexpr decl_free_get_set(std::string_view            iName,
                              free_get_fn<Class, M> iGetter,
                              free_set_fn<Class, M> iSetter)
      : decl_base<Class, M>(iName), free_getter(iGetter),
        free_setter(iSetter)
  {
  }

  void value(Class& obj, M const& value) const
  {
    free_setter(obj, value);
  }

  M const& value(Class const& obj) const
  {
    return free_getter(obj);
  }

protected:
  free_get_fn<Class, M> free_getter = nullptr;
  free_set_fn<Class, M> free_setter = nullptr;
};


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

template <typename Class, typename M>
constexpr auto bind(std::string_view iName, detail::member_ptr<Class, M> iPtr)
{
  return detail::decl_member_ptr<Class, M>(iName, iPtr);
}

template <typename Class, typename M>
constexpr auto bind(std::string_view iName, detail::get_fn<Class, M> iGetter,
                    detail::set_fn<Class, M> iSetter)
{
  return detail::decl_get_set<Class, M>(iName, iGetter, iSetter);
}

template <typename Class, typename M>
constexpr auto bind(std::string_view              iName,
                    detail::free_get_fn<Class, M> iGetter,
                    detail::free_set_fn<Class, M> iSetter)
{
  return detail::decl_free_get_set<Class, M>(iName, iGetter, iSetter);
}

template <typename... Args>
auto json_bind(Args&&... args)
{
  return std::make_tuple(std::forward<Args>(args)...);
}

} // namespace jsb