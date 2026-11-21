// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "calculator.h"
#include <cstdint>
namespace Calculator {
struct IntegerResult { uint64_t value = 0; Error error = Error::None; };
// Unsigned 64-bit words; arithmetic wraps modulo 2^64. Right shift is logical.
IntegerResult evaluateInteger(const std::string& expression, unsigned base = 10);
std::string formatInteger(uint64_t value, unsigned base);
}
