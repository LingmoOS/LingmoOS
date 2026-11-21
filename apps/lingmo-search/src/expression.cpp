// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "expression.h"
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string_view>
#include <numbers>
#include <cctype>
namespace SearchMath {
namespace {
constexpr size_t maxLength = 512;
class Parser {
public:
    Parser(std::string_view text, AngleUnit angle, std::optional<long double> variable)
        : m_text(text), m_angle(angle), m_variable(variable) {}

    Result run()
    {
        const auto value = sum();
        spaces();
        if (m_pos != m_text.size() && m_error == Error::None)
            m_error = Error::InvalidExpression;
        if (!std::isfinite(value) && m_error == Error::None)
            m_error = Error::OutOfRange;
        return {value, m_error};
    }

private:
    void spaces()
    {
        while (m_pos < m_text.size() && std::isspace(static_cast<unsigned char>(m_text[m_pos])))
            ++m_pos;
    }
    bool take(char c)
    {
        spaces();
        if (m_pos < m_text.size() && m_text[m_pos] == c) {
            ++m_pos;
            return true;
        }
        return false;
    }
    long double fail(Error error)
    {
        if (m_error == Error::None)
            m_error = error;
        return 0;
    }
    long double sum()
    {
        auto value = product();
        for (;;) {
            if (take('+')) value += product();
            else if (take('-')) value -= product();
            else return value;
        }
    }
    long double product()
    {
        auto value = unary();
        for (;;) {
            if (take('*')) value *= unary();
            else if (take('/')) {
                const auto rhs = unary();
                if (rhs == 0) return fail(Error::DivisionByZero);
                value /= rhs;
            } else return value;
        }
    }
    // Unary minus binds less tightly than power: -2^2 == -(2^2).
    long double unary()
    {
        if (++m_depth > 64) {
            --m_depth;
            return fail(Error::TooLong);
        }
        long double value;
        if (take('+')) value = unary();
        else if (take('-')) value = -unary();
        else value = power();
        --m_depth;
        return value;
    }
    long double power()
    {
        auto value = postfix();
        if (take('^')) {
            const auto exponent = unary();
            if (value == 0 && exponent < 0) return fail(Error::DivisionByZero);
            if (value < 0 && std::floor(exponent) != exponent) return fail(Error::Domain);
            value = std::pow(value, exponent);
        }
        return value;
    }
    long double postfix()
    {
        auto value = atom();
        for (;;) {
            if (take('%')) value /= 100;
            else if (take('!')) {
                if (value < 0 || std::floor(value) != value) return fail(Error::Domain);
                if (value > 170) return fail(Error::OutOfRange);
                long double factorial = 1;
                for (unsigned i = 2; i <= static_cast<unsigned>(value); ++i) factorial *= i;
                value = factorial;
            } else return value;
        }
    }
    long double function(const std::string& name, long double value)
    {
        const long double radians = m_angle == AngleUnit::Degrees
            ? value * std::numbers::pi_v<long double> / 180 : value;
        const auto inverse = [this](long double result) {
            return m_angle == AngleUnit::Degrees ? result * 180 / std::numbers::pi_v<long double> : result;
        };
        if (name == "sin") return std::sin(radians);
        if (name == "cos") return std::cos(radians);
        if (name == "tan") {
            if (std::abs(std::cos(radians)) < 1e-15L) return fail(Error::Domain);
            return std::tan(radians);
        }
        if (name == "asin" || name == "acos") {
            if (std::abs(value) > 1) return fail(Error::Domain);
            return inverse(name == "asin" ? std::asin(value) : std::acos(value));
        }
        if (name == "atan") return inverse(std::atan(value));
        if (name == "sqrt") return value < 0 ? fail(Error::Domain) : std::sqrt(value);
        if (name == "ln" || name == "log") {
            if (value <= 0) return fail(Error::Domain);
            return name == "ln" ? std::log(value) : std::log10(value);
        }
        if (name == "exp") return std::exp(value);
        if (name == "abs") return std::abs(value);
        return fail(Error::InvalidExpression);
    }
    long double atom()
    {
        if (take('(')) {
            const auto value = sum();
            if (!take(')')) return fail(Error::InvalidExpression);
            return value;
        }
        spaces();
        if (m_pos < m_text.size() && std::isalpha(static_cast<unsigned char>(m_text[m_pos]))) {
            const auto start = m_pos;
            while (m_pos < m_text.size() && std::isalpha(static_cast<unsigned char>(m_text[m_pos]))) ++m_pos;
            const std::string name(m_text.substr(start, m_pos - start));
            if (name == "pi") return std::numbers::pi_v<long double>;
            if (name == "e") return std::numbers::e_v<long double>;
            if (name == "x") return m_variable ? *m_variable : fail(Error::InvalidExpression);
            if (!take('(')) return fail(Error::InvalidExpression);
            const auto value = sum();
            if (!take(')')) return fail(Error::InvalidExpression);
            return function(name, value);
        }
        const auto start = m_pos;
        bool digits = false;
        while (m_pos < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) {
            ++m_pos;
            digits = true;
        }
        if (m_pos < m_text.size() && m_text[m_pos] == '.') {
            ++m_pos;
            while (m_pos < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) {
                ++m_pos;
                digits = true;
            }
        }
        if (!digits) return fail(Error::InvalidExpression);
        if (m_pos < m_text.size() && (m_text[m_pos] == 'e' || m_text[m_pos] == 'E')) {
            ++m_pos;
            if (m_pos < m_text.size() && (m_text[m_pos] == '+' || m_text[m_pos] == '-')) ++m_pos;
            const auto exponent = m_pos;
            while (m_pos < m_text.size() && std::isdigit(static_cast<unsigned char>(m_text[m_pos]))) ++m_pos;
            if (m_pos == exponent) return fail(Error::InvalidExpression);
        }
        std::istringstream stream(std::string(m_text.substr(start, m_pos - start)));
        stream.imbue(std::locale::classic());
        long double value = 0;
        if (!(stream >> value)) return fail(Error::OutOfRange);
        return value;
    }
    std::string_view m_text;
    AngleUnit m_angle;
    std::optional<long double> m_variable;
    size_t m_pos = 0;
    unsigned m_depth = 0;
    Error m_error = Error::None;
};
}
Result evaluate(const std::string& expression, AngleUnit angle, std::optional<long double> x)
{
    if (expression.size() > maxLength) return {0, Error::TooLong};
    return Parser(expression, angle, x).run();
}
std::string format(long double value) {
    if (value == 0) return "0";
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setprecision(15) << value;
    return stream.str();
}
}
