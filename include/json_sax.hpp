//
// Created by obhi on 9/18/20.
//
#pragma once

#include <json_bind.hpp>
#include <memory>
#include <vector>

namespace jsb
{
class json_sax;
class json_sax_base
{
public:
  virtual ~json_sax_base()                 = default;
  virtual bool on_object_begin()           = 0;
  virtual bool on_object_end()             = 0;
  virtual bool on_array_begin()            = 0;
  virtual bool on_array_end()              = 0;
  virtual bool on_key(std::string_view)    = 0;
  virtual bool on_string(std::string_view) = 0;
  virtual bool on_int64(std::int64_t i)    = 0;
  virtual bool on_uint64(std::uint64_t i)  = 0;
  virtual bool on_double(double d)         = 0;
  virtual bool on_bool(bool b)             = 0;
  virtual bool on_null()                   = 0;

private:
  std::unique_ptr<json_sax_base> next;
  friend class json_sax;
};
// Defines a json sax handler that can load an object of a given type.
// This handler is valid for usage with CPPAlliance::boost library
template <typename Class, typename Decl>
class json_sax_object : public json_sax_base
{
public:
  json_sax_object(json_sax& istate_mc, Class& iobject)
      : object(&iobject), state_mc(&istate_mc)
  {
  }
  json_sax_object(json_sax_object const&) = default;
  json_sax_object(json_sax_object&&)      = default;
  json_sax_object& operator=(json_sax_object const&) = default;
  json_sax_object& operator=(json_sax_object&&) = default;

  bool on_object_begin() override;
  bool on_object_end() override;
  bool on_array_begin() override;
  bool on_array_end() override;
  bool on_key(std::string_view) override;
  bool on_string(std::string_view value) override;
  bool on_int64(std::int64_t i) override;
  bool on_uint64(std::uint64_t u) override;
  bool on_double(double d) override;
  bool on_bool(bool b) override;
  bool on_null() override;

  template <typename T>
  bool on_any(T);

private:
  std::string key;

  json_sax* state_mc = nullptr;
  Class*    object   = nullptr;
};

// State machine
class json_sax
{
public:
  template <typename sax_>
  void push_handler_proxy(sax_&& obj)
  {
    auto pobj  = std::make_unique(std::forward<sax_>(obj));
    pobj->next = std::move(top);
    top        = std::move(pobj);
  }

  bool pop_handler()
  {
    if (!top)
      return false;
    top = std::move(top->next);
    return true;
  }

private:
  std::unique_ptr<json_sax_base> top = nullptr;
};
template <typename Class>
inline bool json_sax_object<Class>::on_object_begin()
{
  if (!key.empty())
  {
    detail::for_all(
        [this, &value](Class& obj, auto decl) {
          if (decl.key() == key)
          {
            using type = typename decltype(decl)::MemTy;
            if constexpr (detail::BoundClass<type>)
              decl.set(obj, type(value));
            else if constexpr (detail::CastableFromString<type>)
              decl.set(obj, type(std::string(value)));
            else if constexpr (detail::TransformFromString<type>)
              decl.set(obj, from_string(value));
          }
        },
        *object);
  }
}
template <typename Class>
inline bool json_sax_object<Class>::on_object_end()
{
  if (!state_mc->pop_handler())
    return false;
  return true;
}
template <typename Class>
inline bool json_sax_object<Class>::on_array_begin()
{
  return true;
}
template <typename Class>
inline bool json_sax_object<Class>::on_array_end()
{
  if (!state_mc->pop_handler())
    return false;
  return true;
}
template <typename Class>
inline bool json_sax_object<Class>::on_key(std::string_view k)
{
  key = k;
  return true;
}
template <typename Class>
inline bool json_sax_object<Class>::on_string(std::string_view value)
{
  if (!key.empty())
  {
    detail::for_all(
        [this, &value](Class& obj, auto decl) {
          if (decl.key() == key)
          {
            using type = typename decltype(decl)::MemTy;
            if constexpr (detail::StringType<type> ||
                          detail::CastableFromStringView<type>)
              decl.set(obj, type(value));
            else if constexpr (detail::CastableFromString<type>)
              decl.set(obj, type(std::string(value)));
            else if constexpr (detail::TransformFromString<type>)
              decl.set(obj, from_string(value));
          }
        },
        *object);
  }
  return true;
}
template <typename Class>
inline bool json_sax_object<Class>::on_int64(std::int64_t value)
{
  return on_any(value);
}
template <typename Class>
inline bool json_sax_object<Class>::on_uint64(std::uint64_t value)
{
  return on_any(value);
}
template <typename Class>
inline bool json_sax_object<Class>::on_double(double value)
{
  return on_any(value);
}
template <typename Class>
inline bool json_sax_object<Class>::on_bool(bool value)
{
  return on_any(value);
}
template <typename Class>
inline bool json_sax_object<Class>::on_null()
{
  return true;
}
template <typename Class>
template <typename T>
inline bool json_sax_object<Class>::on_any(T value)
{
  detail::for_all(
      [this, value](Class& obj, auto decl) {
        using type = typename decltype(decl)::MemTy;
        if (decl.key() == key)
        {
          decl.set(obj, type(value));
        }
      },
      *object);
  return true;
}
} // namespace jsb