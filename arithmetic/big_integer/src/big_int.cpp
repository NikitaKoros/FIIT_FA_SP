//
// Created by Des Caldnd on 5/27/2024.
//

#include "../include/big_int.h"
#include <ranges>
#include <exception>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>

std::strong_ordering big_int::operator<=>(const big_int &other) const noexcept
{
    throw not_implemented("std::strong_ordering big_int::operator<=>(const big_int &) const noexcept", "your code should be here...");
}

big_int::operator bool() const noexcept
{
    throw not_implemented("big_int::operator bool()", "your code should be here...");
}

big_int &big_int::operator++() &
{
    throw not_implemented("big_int &big_int::operator++()", "your code should be here...");
}


big_int big_int::operator++(int)
{
    throw not_implemented("big_int big_int::operator++(int)", "your code should be here...");
}

big_int &big_int::operator--() &
{
    throw not_implemented("big_int &big_int::operator--()", "your code should be here...");
}


big_int big_int::operator--(int)
{
    throw not_implemented("big_int big_int::operator--(int)", "your code should be here...");
}

big_int &big_int::operator+=(const big_int &other) &
{
    throw not_implemented("big_int &big_int::operator+=(const big_int &)", "your code should be here...");
}

big_int &big_int::operator-=(const big_int &other) &
{
    throw not_implemented("big_int &big_int::operator-=(const big_int &)", "your code should be here...");
}

big_int big_int::operator+(const big_int &other) const
{
    throw not_implemented("big_int big_int::operator+(const big_int &) const", "your code should be here...");
}

big_int big_int::operator-(const big_int &other) const
{
    throw not_implemented("big_int big_int::operator-(const big_int &) const", "your code should be here...");
}

big_int big_int::operator*(const big_int &other) const
{
    throw not_implemented("big_int big_int::operator*(const big_int &) const", "your code should be here...");
}

big_int big_int::operator/(const big_int &other) const
{
    throw not_implemented("big_int big_int::operator/(const big_int &) const", "your code should be here...");
}

big_int big_int::operator%(const big_int &other) const
{
    throw not_implemented("big_int big_int::operator%(const big_int &) const", "your code should be here...");
}

big_int big_int::operator&(const big_int &other) const
{
    throw not_implemented("big_int big_int::operator&(const big_int &) const", "your code should be here...");
}

big_int big_int::operator|(const big_int &other) const
{
    throw not_implemented("big_int big_int::operator|(const big_int &) const", "your code should be here...");
}

big_int big_int::operator^(const big_int &other) const
{
    throw not_implemented("big_int big_int::operator^(const big_int &) const", "your code should be here...");
}

big_int big_int::operator<<(size_t shift) const
{
    throw not_implemented("big_int big_int::operator<<(size_t ) const", "your code should be here...");
}

big_int big_int::operator>>(size_t shift) const
{
    throw not_implemented("big_int big_int::operator>>(size_t) const", "your code should be here...");
}

big_int &big_int::operator%=(const big_int &other) &
{
    throw not_implemented("big_int &big_int::operator%=(const big_int &)", "your code should be here...");
}

big_int big_int::operator~() const
{
    throw not_implemented("big_int big_int::operator~() const", "your code should be here...");
}

big_int &big_int::operator&=(const big_int &other) &
{
    throw not_implemented("big_int &big_int::operator&=(const big_int &)", "your code should be here...");
}

big_int &big_int::operator|=(const big_int &other) &
{
    throw not_implemented("big_int &big_int::operator|=(const big_int &)", "your code should be here...");
}

big_int &big_int::operator^=(const big_int &other) &
{
    throw not_implemented("big_int &big_int::operator^=(const big_int &)", "your code should be here...");
}

big_int &big_int::operator<<=(size_t shift) &
{
    throw not_implemented("big_int &big_int::operator<<=(size_t)", "your code should be here...");
}

big_int &big_int::operator>>=(size_t shift) &
{
    throw not_implemented("big_int &big_int::operator>>=(size_t)", "your code should be here...");
}

big_int &big_int::plus_assign(const big_int &other, size_t shift) &
{
    throw not_implemented("big_int &big_int::plus_assign(const big_int &, size_t)", "your code should be here...");
}

big_int &big_int::minus_assign(const big_int &other, size_t shift) &
{
    throw not_implemented("big_int &big_int::minus_assign(const big_int &, size_t)", "your code should be here...");
}

big_int &big_int::operator*=(const big_int &other) &
{
    throw not_implemented("big_int &big_int::operator*=(const big_int &)", "your code should be here...");
}

big_int &big_int::operator/=(const big_int &other) &
{
    throw not_implemented("big_int &big_int::operator/=(const big_int &)", "your code should be here...");
}

std::string big_int::to_string() const
{
    throw not_implemented("std::string big_int::to_string() const", "your code should be here...");
}

std::ostream &operator<<(std::ostream &stream, const big_int &value)
{
    throw not_implemented("std::ostream &operator<<(std::ostream &, const big_int &)", "your code should be here...");
}

std::istream &operator>>(std::istream &stream, big_int &value)
{
    throw not_implemented("std::istream &operator>>(std::istream &, big_int &)", "your code should be here...");
}

bool big_int::operator==(const big_int &other) const noexcept
{
    throw not_implemented("bool big_int::operator==(const big_int &) const noexcept", "your code should be here...");
}

big_int::big_int(const std::vector<unsigned int, pp_allocator<unsigned int>> &digits, bool sign)
{
}

big_int::big_int(std::vector<unsigned int, pp_allocator<unsigned int>> &&digits, bool sign) noexcept
{
    throw not_implemented("big_int::big_int(std::vector<unsigned int, pp_allocator<unsigned int>> &&digits, bool sign) noexcept", "your code should be here...");
}

big_int::big_int(const std::string &num, unsigned int radix, pp_allocator<unsigned int>)
{
    if (radix != 10) {
        throw not_implemented("big_int::big_int with radix != 10");
    }

    _sign = true;
    size_t start = 0;
    if (!num.empty()) {
        if (num[0] == '-') {
            _sign = false;
            start = 1;
        } else if (num[0] == '+') {
            start = 1;
        }
    }

    while (start < num.size() && num[start] == '0') {
        ++start;
    }

    if (start == num.size()) {
        _sign = true;
        _digits.clear();
        return;
    }

    std::string digits_str = num.substr(start);
    _digits.push_back(0);

    for (char c : digits_str) {
        if (!isdigit(c)) {
            throw std::invalid_argument("invalid digit in number string");
        }
        unsigned int digit = c - '0';
        multiply_by_digit(10);
        unsigned long long carry = digit;
        for (size_t i = 0; i < _digits.size() && carry > 0; ++i) {
            unsigned long long sum = _digits[i] + carry;
            _digits[i] = static_cast<unsigned int>(sum & 0xFFFFFFFF);
            carry = sum >> 32;
        }
        if (carry > 0) {
            _digits.push_back(static_cast<unsigned int>(carry));
        }
    }

    while (!_digits.empty() && _digits.back() == 0) {
        _digits.pop_back();
    }

    if (_digits.empty()) {
        _sign = true;
    }
}

big_int::big_int(pp_allocator<unsigned int>) :_sign(true), _digits(pp_allocator<unsigned int>()){}

big_int &big_int::multiply_assign(const big_int &other, big_int::multiplication_rule rule) &
{
    throw not_implemented("big_int &big_int::multiply_assign(const big_int &other, big_int::multiplication_rule rule) &", "your code should be here...");
}

big_int &big_int::divide_assign(const big_int &other, big_int::division_rule rule) &
{
    throw not_implemented("big_int &big_int::divide_assign(const big_int &other, big_int::division_rule rule) &", "your code should be here...");
}

big_int &big_int::modulo_assign(const big_int &other, big_int::division_rule rule) &
{
    throw not_implemented("big_int &big_int::modulo_assign(const big_int &other, big_int::division_rule rule) &", "your code should be here...");
}

big_int operator""_bi(unsigned long long n)
{
    throw not_implemented("big_int operator\"\"_bi(unsigned long long n)", "your code should be here...");
}

void big_int::multiply_by_digit(unsigned int digit) {
    unsigned long long carry = 0;
    for (size_t i = 0; i < _digits.size(); ++i) {
        unsigned long long product = (unsigned long long)_digits[i] * digit + carry;
        _digits[i] = static_cast<unsigned int>(product & 0xFFFFFFFF);
        carry = product >> 32;
    }
    if (carry != 0) {
        _digits.push_back(static_cast<unsigned int>(carry));
    }
}