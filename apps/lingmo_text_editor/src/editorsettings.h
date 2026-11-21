// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORSETTINGS_H
#define EDITORSETTINGS_H

#include "base/signal.h"

#include <string>

// The editor's preferences, one instance for the process and every window,
// kept under [Editor] in ~/.config/lingmo/text-editor.conf.
class EditorSettings
{
public:
    explicit EditorSettings(std::string path);

    static std::string defaultPath();
    const std::string &path() const { return m_path; }

    // Empty: the system's monospace font.
    std::string fontFamily;
    // 0: the system font's size.
    double fontSize = 0;
    bool wordWrap = true;
    bool lineNumbers = true;
    bool highlightCurrentLine = false;
    bool showWhitespace = false;
    bool matchBrackets = true;
    bool autoIndent = true;
    int tabWidth = 4;
    bool insertSpaces = true;
    bool restoreSession = true;

    bool findCaseSensitive = false;
    bool findWholeWords = false;
    bool findRegex = false;
    std::string lastFolder;

    // Writes the values and tells every window.
    void commit();

    Lingmo::base::Signal<> changed;

private:
    std::string m_path;
};

#endif
