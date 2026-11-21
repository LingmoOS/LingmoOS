// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "terminalsettings.h"

#include "base/paths.h"
#include "base/settings.h"

TerminalSettings::TerminalSettings(const std::string& path)
    : m_settings(std::make_unique<Lingmo::base::Settings>(path))
{
    m_fontPointSize = int(m_settings->intValue("fontPointSize", m_fontPointSize));
    m_fontName = m_settings->stringValue("fontName", m_fontName);
    m_blinkingCursor = m_settings->boolValue("blinkingCursor", m_blinkingCursor);
    m_cursorShape = int(m_settings->intValue("cursorShape", m_cursorShape));
    m_scrollbackLines = int(m_settings->intValue("scrollbackLines", m_scrollbackLines));
    m_visualBell = m_settings->boolValue("visualBell", m_visualBell);
    m_confirmMultilinePaste = m_settings->boolValue("confirmMultilinePaste", m_confirmMultilinePaste);
    m_colorScheme = m_settings->stringValue("colorScheme", m_colorScheme);
    if (m_colorScheme.empty())
        m_colorScheme = "TokyoNight";
    m_opacity = m_settings->doubleValue("opacity", m_opacity);
    m_blur = m_settings->boolValue("blur", m_blur);
}

TerminalSettings::~TerminalSettings() = default;

std::string TerminalSettings::defaultPath()
{
    return Lingmo::base::paths::lingmoConfig("terminal");
}

void TerminalSettings::save(const std::string& key)
{
    if (key == "fontPointSize")
        m_settings->setValue(key, m_fontPointSize);
    else if (key == "fontName")
        m_settings->setValue(key, m_fontName);
    else if (key == "blinkingCursor")
        m_settings->setValue(key, m_blinkingCursor);
    else if (key == "cursorShape")
        m_settings->setValue(key, m_cursorShape);
    else if (key == "scrollbackLines")
        m_settings->setValue(key, m_scrollbackLines);
    else if (key == "visualBell")
        m_settings->setValue(key, m_visualBell);
    else if (key == "confirmMultilinePaste")
        m_settings->setValue(key, m_confirmMultilinePaste);
    else if (key == "colorScheme")
        m_settings->setValue(key, m_colorScheme);
    else if (key == "opacity")
        m_settings->setValue(key, m_opacity);
    else if (key == "blur")
        m_settings->setValue(key, m_blur);
    m_settings->sync();
    changed();
}

void TerminalSettings::setFontPointSize(int size)
{
    if (size == m_fontPointSize)
        return;
    m_fontPointSize = size;
    save("fontPointSize");
}

void TerminalSettings::setFontName(const std::string& name)
{
    if (name == m_fontName)
        return;
    m_fontName = name;
    save("fontName");
}

void TerminalSettings::setBlinkingCursor(bool blinking)
{
    if (blinking == m_blinkingCursor)
        return;
    m_blinkingCursor = blinking;
    save("blinkingCursor");
}

void TerminalSettings::setCursorShape(int shape)
{
    if (shape == m_cursorShape)
        return;
    m_cursorShape = shape;
    save("cursorShape");
}

void TerminalSettings::setScrollbackLines(int lines)
{
    if (lines == m_scrollbackLines)
        return;
    m_scrollbackLines = lines;
    save("scrollbackLines");
}

void TerminalSettings::setVisualBell(bool visual)
{
    if (visual == m_visualBell)
        return;
    m_visualBell = visual;
    save("visualBell");
}

void TerminalSettings::setConfirmMultilinePaste(bool confirm)
{
    if (confirm == m_confirmMultilinePaste)
        return;
    m_confirmMultilinePaste = confirm;
    save("confirmMultilinePaste");
}

void TerminalSettings::setColorScheme(const std::string& scheme)
{
    if (scheme == m_colorScheme)
        return;
    m_colorScheme = scheme;
    save("colorScheme");
}

void TerminalSettings::setOpacity(double opacity)
{
    if (opacity == m_opacity)
        return;
    m_opacity = opacity;
    save("opacity");
}

void TerminalSettings::setBlur(bool blur)
{
    if (blur == m_blur)
        return;
    m_blur = blur;
    save("blur");
}
