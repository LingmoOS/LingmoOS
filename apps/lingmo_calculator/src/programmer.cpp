// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "programmer.h"
#include <algorithm>
#include <cctype>
#include <limits>
#include <string_view>
namespace Calculator {
namespace {
class IntegerParser {
public:
    IntegerParser(std::string_view text, unsigned base) : m_text(text), m_base(base) {}
    IntegerResult run()
    {
        const auto value = bitOr();
        spaces();
        if (m_pos != m_text.size() && m_error == Error::None) m_error = Error::InvalidExpression;
        return {value, m_error};
    }
private:
    void spaces() { while (m_pos < m_text.size() && std::isspace(static_cast<unsigned char>(m_text[m_pos]))) ++m_pos; }
    bool take(std::string_view text)
    {
        spaces();
        if (m_text.substr(m_pos, text.size()) != text) return false;
        m_pos += text.size();
        return true;
    }
    uint64_t fail(Error error) { if (m_error == Error::None) m_error = error; return 0; }
    uint64_t bitOr() { auto v = bitXor(); while (take("|")) v |= bitXor(); return v; }
    uint64_t bitXor() { auto v = bitAnd(); while (take("^")) v ^= bitAnd(); return v; }
    uint64_t bitAnd() { auto v = shift(); while (take("&")) v &= shift(); return v; }
    uint64_t shift()
    {
        auto value = sum();
        for (;;) {
            const bool left = take("<<");
            if (!left && !take(">>")) return value;
            const auto count = sum();
            if (count >= 64) return fail(Error::Domain);
            value = left ? value << count : value >> count;
        }
    }
    uint64_t sum()
    {
        auto value = product();
        for (;;) {
            if (take("+")) value += product();
            else if (take("-")) value -= product();
            else return value;
        }
    }
    uint64_t product()
    {
        auto value = unary();
        for (;;) {
            if (take("*")) value *= unary();
            else if (take("/")) { const auto rhs = unary(); if (!rhs) return fail(Error::DivisionByZero); value /= rhs; }
            else if (take("%")) { const auto rhs = unary(); if (!rhs) return fail(Error::DivisionByZero); value %= rhs; }
            else return value;
        }
    }
    uint64_t unary()
    {
        if (++m_depth > 64) { --m_depth; return fail(Error::TooLong); }
        uint64_t value;
        if (take("~")) value = ~unary();
        else if (take("-")) value = uint64_t(0) - unary();
        else if (take("+")) value = unary();
        else if (take("(")) { value = bitOr(); if (!take(")")) fail(Error::InvalidExpression); }
        else value = number();
        --m_depth;
        return value;
    }
    uint64_t number()
    {
        spaces();
        unsigned base = m_base;
        if (m_text.substr(m_pos, 2) == "0x" || m_text.substr(m_pos, 2) == "0X") { base = 16; m_pos += 2; }
        else if (m_text.substr(m_pos, 2) == "0b" || m_text.substr(m_pos, 2) == "0B") { base = 2; m_pos += 2; }
        else if (m_text.substr(m_pos, 2) == "0o" || m_text.substr(m_pos, 2) == "0O") { base = 8; m_pos += 2; }
        const auto start = m_pos;
        uint64_t value = 0;
        while (m_pos < m_text.size()) {
            const unsigned char c = static_cast<unsigned char>(m_text[m_pos]);
            const unsigned digit = c >= '0' && c <= '9' ? c - '0'
                : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : 99;
            if (digit == 99) break;
            if (digit >= base) return fail(Error::InvalidExpression);
            if (value > (std::numeric_limits<uint64_t>::max() - digit) / base) return fail(Error::OutOfRange);
            value = value * base + digit;
            ++m_pos;
        }
        if (start == m_pos) return fail(Error::InvalidExpression);
        return value;
    }
    std::string_view m_text;
    unsigned m_base;
    size_t m_pos = 0;
    unsigned m_depth = 0;
    Error m_error = Error::None;
};
}
IntegerResult evaluateInteger(const std::string& expression, unsigned base)
{
    if (expression.size() > 512) return {0, Error::TooLong};
    if (base != 2 && base != 8 && base != 10 && base != 16) return {0, Error::InvalidExpression};
    return IntegerParser(expression, base).run();
}
std::string formatInteger(uint64_t value, unsigned base)
{
    if (base != 2 && base != 8 && base != 10 && base != 16) return {};
    std::string result;
    do { result += "0123456789ABCDEF"[value % base]; value /= base; } while (value);
    std::reverse(result.begin(), result.end());
    return result;
}
}
