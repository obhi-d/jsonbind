//
// Created by obhi on 9/18/20.
//
#pragma once

#include <json_bind.hpp>

namespace jsb
{

// json_value from a given API is transformed into
// a bound class using this interface
template <typename Class>
requires (detail::is_class_bound<Class>)
class json_vstream
{

};


}