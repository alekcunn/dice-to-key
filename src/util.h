#pragma once
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/integer/common_factor.hpp>

using boost::multiprecision::cpp_int;
using boost::multiprecision::uint1024_t;


cpp_int carmichael_fixed(const uint1024_t& p, const uint1024_t& q);
cpp_int mod_inverse(const cpp_int& a, const cpp_int& m);