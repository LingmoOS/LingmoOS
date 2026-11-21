// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <string>
#include <optional>
#include <vector>
namespace SearchMath {
enum class Error { None, InvalidExpression, DivisionByZero, OutOfRange, TooLong, Domain };
struct Result { long double value = 0; Error error = Error::None; };
enum class AngleUnit { Radians, Degrees };
Result evaluate(const std::string& expression, AngleUnit angle = AngleUnit::Radians, std::optional<long double> x = {});
std::string format(long double value);
}
