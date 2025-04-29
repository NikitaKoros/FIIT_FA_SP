#include "../include/fraction.h"

void fraction::optimise()
{
    if (_numerator == 0) {
        _denominator = 1;
        return;
    }
    
    big_int divisor = gcd(_numerator.abs(), _denominator.abs());
    
    _numerator /= divisor;
    _denominator /= divisor;
    
    if (_denominator < 0) {
        _numerator = -_numerator;
        _denominator = -_denominator;
    }
}

fraction::fraction(pp_allocator<big_int::value_type> allocator)
    : _numerator(0, allocator), _denominator(1, allocator) {}

fraction &fraction::operator+=(fraction const &other) &
{
    if (_denominator == other._denominator) {
        _numerator += other._numerator;
    } else {
        big_int lcm_value = lcm(_denominator, other._denominator);
        
        _numerator = _numerator * (lcm_value / _denominator) + 
                     other._numerator * (lcm_value / other._denominator);
        _denominator = lcm_value;
    }
    
    optimise();
    return *this;
}

fraction fraction::operator+(fraction const &other) const
{
    fraction result = *this;
    result += other;
    return result;
}

fraction &fraction::operator-=(fraction const &other) &
{
    if (_denominator == other._denominator) {
        _numerator -= other._numerator;
    } else {
        big_int lcm_value = lcm(_denominator, other._denominator);
        
        _numerator = _numerator * (lcm_value / _denominator) - 
                     other._numerator * (lcm_value / other._denominator);
        _denominator = lcm_value;
    }
    
    optimise();
    return *this;
}

fraction fraction::operator-(fraction const &other) const
{
    fraction result = *this;
    result -= other;
    return result;
}

fraction &fraction::operator*=(fraction const &other) &
{
    _numerator *= other._numerator;
    _denominator *= other._denominator;
    
    optimise();
    return *this;
}

fraction fraction::operator*(fraction const &other) const
{
    fraction result = *this;
    result *= other;
    return result;
}

fraction operator*(int lhs, const fraction& rhs) {
    return fraction(lhs) * rhs;
}

fraction &fraction::operator/=(fraction const &other) &
{
    if (other._numerator == 0) {
        throw std::invalid_argument("Division by zero");
    }
    
    _numerator *= other._denominator;
    _denominator *= other._numerator;
    
    optimise();
    return *this;
}

fraction fraction::operator/(fraction const &other) const
{
    fraction result = *this;
    result /= other;
    return result;
}

bool fraction::operator==(fraction const &other) const noexcept
{
    return _numerator == other._numerator && _denominator == other._denominator;
}

std::partial_ordering fraction::operator<=>(const fraction& other) const noexcept
{
    big_int left = _numerator * other._denominator;
    big_int right = other._numerator * _denominator;
    
    bool same_sign = (_denominator > 0 && other._denominator > 0) || 
                     (_denominator < 0 && other._denominator < 0);
    
    if (same_sign) {
        if (left < right) return std::partial_ordering::less;
        if (left > right) return std::partial_ordering::greater;
        return std::partial_ordering::equivalent;
    } else {
        if (left > right) return std::partial_ordering::less;
        if (left < right) return std::partial_ordering::greater;
        return std::partial_ordering::equivalent;
    }
}

std::ostream &operator<<(std::ostream &stream, fraction const &obj)
{
    if (obj._denominator == 1) {
        stream << obj._numerator;
    } else {
        stream << obj._numerator << "/" << obj._denominator;
    }
    return stream;
}

std::istream &operator>>(std::istream &stream, fraction &obj)
{
    std::string input;
    stream >> input;
    
    size_t slash_pos = input.find('/');
    if (slash_pos != std::string::npos) {
        // numerator/denominator
        std::string num_str = input.substr(0, slash_pos);
        std::string den_str = input.substr(slash_pos + 1);
        
        big_int numerator(num_str);
        big_int denominator(den_str);
        
        if (denominator == 0) {
            stream.setstate(std::ios::failbit);
            return stream;
        }
        
        obj._numerator = numerator;
        obj._denominator = denominator;
    } else {
        try {
            big_int value(input);
            obj._numerator = value;
            obj._denominator = 1;
        } catch (const std::exception& e) {
            stream.setstate(std::ios::failbit);
            return stream;
        }
    }
    obj.optimise();
    return stream;
}

std::string fraction::to_string() const
{
    if (_denominator == 1) {
        return _numerator.to_string();
    }
    
    std::string result = _numerator.to_string() + "/" + _denominator.to_string();
    return result;
}

fraction fraction::sin(fraction const &epsilon) const
{
    fraction x = *this;
    fraction sum;
    fraction term = x;
    big_int k(1);
    sum = term;

    do {
        term = term * (-(x * x)) / fraction((2 * k) * (2 * k + 1));
        sum += term;
        k += 1;
    } while (term.abs() > epsilon);

    return sum;
}

fraction fraction::cos(fraction const &epsilon) const
{
    fraction x = *this;
    fraction sum(1);
    fraction term(1);
    big_int k(1);

    do {
        term = term * (-(x * x)) / fraction((2 * k - 1) * (2 * k));
        sum += term;
        k += 1;
    } while (term.abs() > epsilon);

    return sum;
}

fraction fraction::tg(fraction const &epsilon) const
{
    fraction s = sin(epsilon);
    fraction c = cos(epsilon);
    if (c == fraction(0)) {
        throw std::domain_error("Tangens is undefined for this angle");
    }
    return s / c;
}

fraction fraction::ctg(fraction const &epsilon) const
{
    fraction c = cos(epsilon);
    fraction s = sin(epsilon);
    if (s == fraction(0)) {
        throw std::domain_error("Cotangent is undefined for this angle");
    }
    return c / s;
}

fraction fraction::sec(fraction const &epsilon) const
{
    fraction c = cos(epsilon);
    if (c == fraction(0)) {
        throw std::domain_error("Secant is undefined for this angle");
    }
    return fraction(1) / c;
}

fraction fraction::cosec(fraction const &epsilon) const
{
    fraction s = sin(epsilon);
    if (s == fraction(0)) {
        throw std::domain_error("Cosecant is undefined for this angle");
    }
    return fraction(1) / s;
}

fraction fraction::arcsin(fraction const &epsilon) const {
    fraction x = *this;
    if (x.abs() > fraction(1)) {
        throw std::invalid_argument("arcsin argument must be in [-1, 1]");
    }

    fraction sum = x;
    fraction term = x;
    big_int n(1);

    do {
        fraction coeff = fraction((2 * n - 1) * (2 * n - 1), 2 * n * (2 * n + 1));
        term = term * x * x * coeff;
        if (term.abs() <= epsilon) {
            break;
        }
        sum += term;
        n += 1;
    } while (true);

    return sum;
}

fraction fraction::arctg(fraction const &epsilon) const {
    fraction x = *this;
    bool is_reciprocal = false;
    if (x.abs() > fraction(1)) {
        x = fraction(1) / x;
        is_reciprocal = true;
    }

    fraction sum = x;
    fraction term = x;
    big_int n(1);
    fraction next_term(0);
    do {
        term = term * (-(x * x));
        fraction next_term = term / fraction(2 * n + 1);
        sum += next_term;
        n += 1;
    } while (next_term.abs() > epsilon);

    if (is_reciprocal) {
        fraction pi_2 = fraction(1).arcsin(fraction(1_bi, 1000000_bi)) * fraction(2);
        sum = pi_2 - sum;
    }

    return sum;
}

fraction fraction::arccos(fraction const &epsilon) const {
    if (*this == fraction(1)) {
        return fraction(0);
    }
    if (*this == fraction(-1)) {
        fraction pi = fraction(1).arcsin(epsilon) * fraction(2) * fraction(2);
        return pi;
    }
    
    fraction pi_2 = fraction(1).arcsin(epsilon) * fraction(2);
    return pi_2 - this->arcsin(epsilon);
}

fraction fraction::arcctg(fraction const &epsilon) const {
    fraction x = *this;
    bool is_reciprocal = false;
    if (x.abs() > fraction(1)) {
        x = fraction(1) / x;
        is_reciprocal = true;
    }

    fraction sum = x;
    fraction term = x;
    big_int n(1);
    fraction next_term;
    do {
        term = term * (-(x * x));
        next_term = term / fraction(2 * n + 1);
        sum += next_term;
        n += 1;
    } while (next_term.abs() > epsilon);

    if (is_reciprocal) {
        fraction pi_2 = fraction(1).arcsin(epsilon) * fraction(2);
        sum = pi_2 - sum;
    }

    return sum;
}

fraction fraction::arcsec(fraction const &epsilon) const {
    if (this->abs() < fraction(1)) {
        throw std::invalid_argument("arcsec argument must be |x| >= 1");
    }
    
    if (*this == fraction(1)) {
        return fraction(0);
    }
    if (*this == fraction(-1)) {
        fraction pi = fraction(1).arcsin(epsilon) * fraction(2) * fraction(2);
        return pi;
    }
    
    fraction inv = fraction(1) / *this;
    
    fraction pi_2 = fraction(1).arcsin(epsilon) * fraction(2);
    return pi_2 - inv.arcsin(epsilon);
}

fraction fraction::arccosec(fraction const &epsilon) const {
    if (this->abs() < fraction(1)) {
        throw std::invalid_argument("arccosec argument must be |x| >= 1");
    }
    
    if (*this == fraction(1)) {
        return fraction(1).arcsin(epsilon) * fraction(2);
    }
    if (*this == fraction(-1)) {
        return fraction(-1) * fraction(1).arcsin(epsilon) * fraction(2);
    }
    
    fraction inv = fraction(1) / *this;
    return inv.arcsin(epsilon);
}

fraction fraction::pow(size_t degree) const
{
    if (degree == 0) {
        return fraction(1, 1);
    }
    
    fraction result(1, 1);
    fraction base = *this;
    
    while (degree > 0) {
        if (degree & 1) {
            result *= base;
        }
        base *= base;
        degree >>= 1;
    }
    
    result.optimise();
    return result;
}

fraction fraction::root(size_t degree, fraction const &epsilon) const
{
    if (degree == 0) 
    {
        throw std::invalid_argument("Root degree cannot be zero");
    }
    if (degree % 2 == 0 && *this < fraction(0)) 
    {
        throw std::invalid_argument("Even root of negative number");
    }
    if (*this == fraction(0)) 
    {
        return fraction(0);
    }

    fraction abs_this = this->abs();
    fraction x_prev = fraction(1);
    fraction x_next;

    while (true) 
    {
        fraction pow_prev = x_prev.pow(degree - 1);
        pow_prev.optimise();
        
        fraction term = abs_this / pow_prev;
        term.optimise();
        
        x_next = (x_prev * fraction(degree - 1) + term) / fraction(degree);
        x_next.optimise();
        
        fraction diff = (x_next - x_prev).abs();
        diff.optimise();

        fraction rel_diff = diff / x_next.abs();
        rel_diff.optimise();
        
        if (rel_diff <= epsilon || x_next == x_prev) 
        {
            break;
        }
        x_prev = x_next;
    }

    fraction result = x_next;
    result.optimise();

    //big_int gcd_val = gcd(result._numerator.abs(), result._denominator.abs());
    
    return result;
}

fraction fraction::log2(fraction const &epsilon) const
{
    fraction ln_x = ln(epsilon);
    fraction ln_2 = fraction(2).ln(epsilon);
    return ln_x / ln_2;
}

fraction fraction::ln(fraction const &epsilon) const
{
    if (*this <= fraction(0)) {
        throw std::invalid_argument("Logarithm of non-positive number");
    }

    fraction x = *this;
    fraction y = (x - 1) / (x + 1);
    fraction sum = y;
    fraction term = y;
    big_int k(1);
    fraction increment(0);
    do {
        term = term * y * y;
        fraction increment = term / fraction(2 * k + 1);
        sum += increment;
        k += 1;
    } while (increment.abs() > epsilon);

    return 2 * sum;
}

fraction fraction::lg(fraction const &epsilon) const
{
    fraction ln_x = ln(epsilon);
    fraction ln_10 = fraction(10).ln(epsilon);
    return ln_x / ln_10;
}

big_int gcd(big_int a, big_int b) {
    if (a == 0) return b;
    if (b == 0) return a;

    a = a < 0 ? -a : a;
    b = b < 0 ? -b : b;
    
    while (b) {
        big_int temp = b;
        b = a % b;
        a = temp;
    }
    
    return a;
}

big_int lcm(const big_int& a, const big_int& b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    
    big_int abs_a = a < 0 ? -a : a;
    big_int abs_b = b < 0 ? -b : b;
    
    return (abs_a / gcd(abs_a, abs_b)) * abs_b;
}

fraction fraction::abs() const {
    return fraction(_numerator.abs(), _denominator.abs());
}

fraction fraction::operator-() const {
    return fraction(-_numerator, _denominator);
}

