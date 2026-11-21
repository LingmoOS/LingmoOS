// Copyright 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace Lingmo::Shell {
struct ScreenKey {
    std::string id, label;
    uint32_t symbol = 0;
    double width = 1;
};
// UI modifiers are one-shot intentions, never held down in the seat between taps.
class ScreenKeyboardModel {
public:
    std::vector<std::vector<ScreenKey>> rows() const;
    bool toggle(const std::string& id);
    void consumed();
    void reset();
    bool selected(const std::string& id) const;
    bool control() const { return m_control; }
    bool alt() const { return m_alt; }
private:
    bool m_shift = false, m_caps = false, m_control = false, m_alt = false, m_symbols = false;
};
// Exclude already held physical/client keys so a tap cannot release somebody
// else's modifier. If the main key is already held, do not inject the chord.
std::vector<uint32_t> screenKeyboardChord(uint32_t key, const std::vector<uint32_t>& modifiers,
                                        bool control, bool alt, const std::vector<uint32_t>& held);
}
