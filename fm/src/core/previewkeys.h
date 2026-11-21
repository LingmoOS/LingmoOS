// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

// Quick Look's D-Bus interface carries keys as codes (letters upper case,
// special keys from 0x01000000) and modifiers as bits from 0x02000000 up.
namespace fm::previewkeys {

// 0 for a key the interface does not carry.
int toWireKey(uint32_t keysym);
uint32_t fromWireKey(int key);
int toWireModifiers(uint32_t modifiers);
uint32_t fromWireModifiers(int modifiers);

} // namespace fm::previewkeys
