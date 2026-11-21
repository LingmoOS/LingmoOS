// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <string>
#include <optional>
#include <vector>
namespace Calculator {
enum class Error { None, InvalidExpression, DivisionByZero, OutOfRange, TooLong, Domain };
struct Result { long double value = 0; Error error = Error::None; };
enum class AngleUnit { Radians, Degrees };
Result evaluate(const std::string& expression, AngleUnit angle = AngleUnit::Radians, std::optional<long double> x = {});
std::string format(long double value);
struct HistoryEntry {
    std::string expression;
    std::string result;
    AngleUnit angle = AngleUnit::Radians;
    bool integer = false;
    unsigned base = 10;
};
class Model {
public:
    void input(const std::string& text);
    bool paste(const std::string& text);
    void backspace();
    void clear();
    void negate();
    bool calculate();
    bool recall(size_t index);
    void addIntegerHistory(const std::string& expression, const std::string& result, unsigned base);
    std::string preview() const;
    void setAngleUnit(AngleUnit angle) { m_angle = angle; }
    AngleUnit angleUnit() const { return m_angle; }
    const std::string& calculation() const { return m_calculation; }
    void clearHistory() { m_history.clear(); }
    const std::string& expression() const { return m_expression; }
    const std::vector<HistoryEntry>& history() const { return m_history; }
    Error error() const { return m_error; }
    bool completed() const { return m_completed; }
private:
    std::string m_expression;
    std::string m_calculation;
    std::vector<HistoryEntry> m_history;
    Error m_error = Error::None;
    bool m_completed = false;
    AngleUnit m_angle = AngleUnit::Radians;
};
}
