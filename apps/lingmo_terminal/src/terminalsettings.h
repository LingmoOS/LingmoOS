// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <string>

#include "base/signal.h"

namespace Lingmo::base {
class Settings;
}

// The terminal's settings in ~/.config/lingmo/terminal.conf, one object for
// every window of the process, so a change shows in all of them at once.
class TerminalSettings
{
public:
    explicit TerminalSettings(const std::string& path);
    ~TerminalSettings();

    static std::string defaultPath();

    int fontPointSize() const { return m_fontPointSize; }
    const std::string& fontName() const { return m_fontName; }
    bool blinkingCursor() const { return m_blinkingCursor; }
    // 0 block, 1 underline, 2 I-beam.
    int cursorShape() const { return m_cursorShape; }
    // Lines kept above the screen; -1 keeps everything.
    int scrollbackLines() const { return m_scrollbackLines; }
    bool visualBell() const { return m_visualBell; }
    bool confirmMultilinePaste() const { return m_confirmMultilinePaste; }
    // A bundled colour scheme, by file name; an empty one from before themes
    // existed reads as TokyoNight.
    const std::string& colorScheme() const { return m_colorScheme; }
    double opacity() const { return m_opacity; }
    bool blur() const { return m_blur; }

    void setFontPointSize(int size);
    void setFontName(const std::string& name);
    void setBlinkingCursor(bool blinking);
    void setCursorShape(int shape);
    void setScrollbackLines(int lines);
    void setVisualBell(bool visual);
    void setConfirmMultilinePaste(bool confirm);
    void setColorScheme(const std::string& scheme);
    void setOpacity(double opacity);
    void setBlur(bool blur);

    // After any setting changed.
    Lingmo::base::Signal<> changed;

private:
    void save(const std::string& key);

    std::unique_ptr<Lingmo::base::Settings> m_settings;
    int m_fontPointSize = 10;
    std::string m_fontName = "Noto Mono";
    bool m_blinkingCursor = true;
    int m_cursorShape = 0;
    int m_scrollbackLines = 10000;
    bool m_visualBell = false;
    bool m_confirmMultilinePaste = true;
    std::string m_colorScheme = "TokyoNight";
    double m_opacity = 0.85;
    bool m_blur = true;
};
