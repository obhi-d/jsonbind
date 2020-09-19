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
  using ClassTy = std::decay_t<Class>;
  using MemTy   = std::decay_t<M>;

  constexpr decl_base(std::string_view iName) : name(iName) {}

  std::string_view key() const
  {
    return name;
  }

private:
  std::string_view name = nullptr;
};

template <typename Class, auto MPtr>
class decl_member_ptr
    : public decl_base<Class, typename detail::member_ptr_t<MPtr>::member_t>
{
public:
  using base_t =
      decl_base<Class, typename detail::member_ptr_t<MPtr>::member_t>;
  using M = typename base_t::MemTy;

  constexpr decl_member_ptr(std::string_view iName) : decl_base<Class, M>(iName)
  {
  }

  inline void value(Class& obj, M const& value) const
  {
    obj.*MPtr = value;
  }

  inline void value(Class& obj, M&& value) const
  {
    obj.*MPtr = std::move(value);
  }

  M const& value(Class const& obj) const
  {
    return (obj.*MPtr);
  }
};

template <typename Class, auto Getter, auto Setter>
class decl_get_set
    : public decl_base<Class,
                       typename detail::member_getter_t<Getter>::return_t>
{
public:
  using base_t =
      decl_base<Class, typename detail::member_getter_t<Getter>::return_t>;
  using M = typename base_t::MemTy;

  constexpr decl_get_set(std::string_view iName) : base_t(iName) {}

  inline void value(Class& obj, M&& value) const
  {
    (obj.*Setter)(std::move(value));
  }

  auto value(Class const& obj) const
  {
    return ((obj.*Getter)());
  }
};

template <typename Class, auto Getter, auto Setter>
class decl_free_get_set
    : public decl_base<Class, typename detail::free_getter_t<Getter>::return_t>
{
public:
  using base_t =
      decl_base<Class, typename detail::free_getter_t<Getter>::return_t>;
  using M = typename base_t::MemTy;

  constexpr decl_free_get_set(std::string_view iName) : base_t(iName) {}

  void value(Class& obj, M const& value) const
  {
    (*Setter)(obj, value);
  }

  auto value(Class const& obj) const
  {
    return (*Getter)(obj);
  }
};

template <typename Class>
auto const& get_decl()
{
  static const auto json_bind_ = decl<std::decay_t<Class>>();
  return json_bind_;
}

template <typename TupleTy, typename Class, typename Fn, size_t... I>
void apply_set(Fn&& fn, Class& obj, TupleTy&& tup, std::index_sequence<I...>)
{
  (fn(obj, std::get<I>(tup)), ...);
}

template <typename TupleTy, typename Class, typename Fn, size_t... I>
void apply_get(Fn&& fn, Class const& obj, TupleTy&& tup,
               std::index_sequence<I...>)
{
  (fn(obj, std::get<I>(tup)), ...);
}

template <typename Class, typename Fn>
void set_all(Fn&& fn, Class& obj)
{
  apply_set(std::forward<Fn>(fn), obj, get_decl<Class>(),
            std::make_index_sequence<tuple_size<Class>>());
}

template <typename Class, typename Fn>
void get_all(Fn&& fn, Class const& obj)
{
  apply_get(std::forward<Fn>(fn), obj, get_decl<Class>(),
            std::make_index_sequence<tuple_size<Class>>());
}

} // namespace detail

template <auto MPtr>
requires(detail::IsMemberPtr<MPtr>) constexpr auto bind(std::string_view iName)
{
  return detail::decl_member_ptr<typename detail::member_ptr_t<MPtr>::class_t,
                                 MPtr>(iName);
}

template <auto Getter, auto Setter>
requires(detail::IsMemberGetterSetter<Getter, Setter>) constexpr auto bind(
    std::string_view iName)
{
  return detail::decl_get_set<typename detail::member_getter_t<Getter>::class_t,
                              Getter, Setter>(iName);
}

template <auto Getter, auto Setter>
requires(detail::IsFreeGetterSetter<Getter, Setter>) constexpr auto bind(
    std::string_view iName)
{
  return detail::decl_free_get_set<
      typename detail::free_getter_t<Getter>::class_t, Getter, Setter>(iName);
}

template <typename... Args>
auto json_bind(Args&&... args)
{
  return std::make_tuple(std::forward<Args>(args)...);
}

} // namespace jsb