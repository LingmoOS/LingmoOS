// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "calculator.h"
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string_view>
#include <numbers>
#include <cctype>
namespace Calculator {
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
void Model::input(const std::string& text)
{
    if (text.empty() || text.find_first_not_of("0123456789.+-*/()%!^abcdefghijklmnopqrstuvwxyzE") != std::string::npos)
        return;

    std::string next = m_expression;
    const bool newNumber = text.front() == '(' || text.front() == '.'
        || (std::isalpha(static_cast<unsigned char>(text.front())) && text.front() != 'E')
        || (text.front() >= '0' && text.front() <= '9');
    if (m_completed && newNumber)
        next.clear();

    // Interactive single-key corrections; pasted expressions retain their syntax.
    if (text == ".") {
        size_t numberStart = next.size();
        while (numberStart > 0) {
            const char previous = next[numberStart - 1];
            if (std::string("+-*/()%").find(previous) != std::string::npos) {
                if ((previous != '+' && previous != '-') || numberStart < 2
                    || (next[numberStart - 2] != 'e' && next[numberStart - 2] != 'E'))
                    break;
            }
            --numberStart;
        }
        const auto number = next.substr(numberStart);
        if (number.find_first_of(".eE") != std::string::npos)
            return;
        if (number.empty())
            next += '0';
    }
    if (text.size() == 1 && text.find_first_of("+*/") != std::string::npos) {
        if (next.empty())
            return;
        // Replace a pending binary operator, without changing exponent signs.
        if (next.size() > 1 && std::string("+-*/").find(next.back()) != std::string::npos
            && next[next.size() - 2] != 'e' && next[next.size() - 2] != 'E') {
            next.pop_back();
        }
    }
    if (next.size() + text.size() > maxLength) {
        m_error = Error::TooLong;
        return;
    }
    m_expression = next + text;
    m_completed = false;
    m_error = Error::None;
}

bool Model::paste(const std::string& text)
{
    // Check the byte limit before normalizing clipboard input.
    if (text.size() > maxLength * 3) {
        m_error = Error::TooLong;
        return false;
    }
    std::string normalized = text;
    for (const auto& [symbol, replacement] : {
             std::pair<std::string, std::string>{"×", "*"}, {"÷", "/"}, {"−", "-"}}) {
        size_t position = 0;
        while ((position = normalized.find(symbol, position)) != std::string::npos) {
            normalized.replace(position, symbol.size(), replacement);
            position += replacement.size();
        }
    }
    const auto first = normalized.find_first_not_of(" \t\r\n");
    const auto last = normalized.find_last_not_of(" \t\r\n");
    if (first != std::string::npos)
        normalized = normalized.substr(first, last - first + 1);
    if (normalized.size() > maxLength) {
        m_error = Error::TooLong;
        return false;
    }
    if (first == std::string::npos || normalized.find_first_not_of("0123456789.+-*/()%!^abcdefghijklmnopqrstuvwxyzE \t\r\n") != std::string::npos) {
        m_error = Error::InvalidExpression;
        return false;
    }
    m_expression = std::move(normalized);
    m_completed = false;
    m_error = Error::None;
    return true;
}

std::string Model::preview() const
{
    if (m_completed || m_error != Error::None || m_expression.empty())
        return {};
    const auto result = evaluate(m_expression, m_angle);
    if (result.error != Error::None)
        return {};
    const auto answer = format(result.value);
    return answer == m_expression ? std::string() : answer;
}

bool Model::recall(size_t index)
{
    if (index >= m_history.size() || m_history[index].integer)
        return false;
    m_expression = m_history[index].result;
    m_calculation = m_history[index].expression;
    m_angle = m_history[index].angle;
    m_completed = true;
    m_error = Error::None;
    return true;
}

void Model::addIntegerHistory(const std::string& expression, const std::string& result, unsigned base)
{
    m_history.insert(m_history.begin(), {expression, result, AngleUnit::Radians, true, base});
    if (m_history.size() > 50) m_history.pop_back();
}

void Model::backspace() { if (!m_expression.empty()) m_expression.pop_back(); m_completed = false; m_error = Error::None; }
void Model::clear() { m_calculation.clear(); m_expression.clear(); m_completed = false; m_error = Error::None; }
void Model::negate() {
    if (m_expression.empty()) { input("-"); return; }
    if (m_expression.size() + 3 > maxLength) { m_error = Error::TooLong; return; }
    m_expression = "-(" + m_expression + ")"; m_completed = false; m_error = Error::None;
}
bool Model::calculate() {
    if (m_completed) return true;
    const auto result = evaluate(m_expression, m_angle);
    m_error = result.error;
    if (m_error != Error::None) return false;
    auto answer = format(result.value);
    m_calculation = m_expression;
    m_history.insert(m_history.begin(), {m_expression, answer, m_angle});
    if (m_history.size() > 50) m_history.pop_back();
    m_expression = std::move(answer); m_completed = true; return true;
}
}
