//
// Created by obhi on 9/18/20.
//
#pragma once

#include <json_bind.hpp>

namespace jsb
{

class json_sax_base
{
public:
  virtual bool on_object_begin() = 0;
  virtual bool on_object_end() = 0;
  virtual bool on_array_begin() = 0;
  virtual bool on_array_end() = 0;
  virtual bool on_key(std::string_view) = 0;
  virtual bool on_string(std::string_view) = 0;
  virtual bool on_int64( std::int64_t i ) = 0;
  virtual bool on_uint64( std::uint64_t i ) = 0;
  virtual bool on_double( double d ) = 0;
  virtual bool on_bool(bool b) = 0;
  virtual bool on_null() = 0;
};

struct json_sax;
// Defines a json sax handler that can load an object of a given type.
// This handler is valid for usage with CPPAlliance::boost library
template <typename Class>
requires (detail::is_class_bound<Class>)
class json_sax_object : public json_sax_base
{
public:
  json_sax_object(json_sax& istate_mc, Class& iobject) : object(iobject), state_mc(istate_mc) {}


  bool on_object_begin() override;
  bool on_object_end() override;
  bool on_array_begin() override;
  bool on_array_end() override;
  bool on_key(std::string_view) override;
  bool on_string(std::string_view s) override;
  bool on_int64(std::int64_t i) override;
  bool on_uint64(std::uint64_t u) override;
  bool on_double( double d ) override;
  bool on_bool( bool b ) override;
  bool on_null() override;

private:
  json_sax& state_mc;
  Class& object;
};



}
