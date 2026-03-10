#include <iostream>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/integer/common_factor.hpp>

using namespace boost::multiprecision;

// Using the typedef we discussed earlier
typedef number<cpp_int_backend<2048, 2048, unsigned_magnitude, unchecked, void>> uint2048_t;

// O(1) Carmichael Lambda for n = p * q
cpp_int carmichael_fixed(const uint1024_t& p, const uint1024_t& q) {
    cpp_int p_minus_1 = p - 1;
    cpp_int q_minus_1 = q - 1;
    
    // lambda(n) = lcm(p-1, q-1)
    // lcm(a, b) = (a * b) / gcd(a, b)
    return (p_minus_1 * q_minus_1) / boost::integer::gcd(p_minus_1, q_minus_1);
}

// Extended Euclidean Algorithm for modular inverse
cpp_int mod_inverse(const cpp_int& a, const cpp_int& m) {
    cpp_int m0 = m, x0 = 0, x1 = 1;
    if (m == 1) return 0;
    cpp_int a_copy = a;
    while (a_copy > 1) {
        cpp_int q = a_copy / m0;
        cpp_int t = m0;
        m0 = a_copy % m0;
        a_copy = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    if (x1 < 0) x1 += m;
    return x1;
}