#pragma once
#include <boost/multiprecision/cpp_int.hpp>

using boost::multiprecision::cpp_int;

void save_pem(const cpp_int& n, const cpp_int& e, const cpp_int& d, 
              const cpp_int& p, const cpp_int& q);