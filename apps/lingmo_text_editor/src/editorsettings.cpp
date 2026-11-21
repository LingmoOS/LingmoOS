// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorsettings.h"

#include "base/paths.h"
#include "base/settings.h"

#include <algorithm>

using Lingmo::base::Settings;

EditorSettings::EditorSettings(std::string path)
    : m_path(std::move(path))
{
    const Settings settings(m_path);
    fontFamily = settings.stringValue("Editor/fontFamily", fontFamily);
    fontSize = settings.doubleValue("Editor/fontSize", fontSize);
    wordWrap = settings.boolValue("Editor/wordWrap", wordWrap);
    lineNumbers = settings.boolValue("Editor/lineNumbers", lineNumbers);
    highlightCurrentLine = settings.boolValue("Editor/highlightCurrentLine", highlightCurrentLine);
    showWhitespace = settings.boolValue("Editor/showWhitespace", showWhitespace);
    matchBrackets = settings.boolValue("Editor/matchBrackets", matchBrackets);
    autoIndent = settings.boolValue("Editor/autoIndent", autoIndent);
    tabWidth = int(std::clamp<int64_t>(settings.intValue("Editor/tabWidth", tabWidth), 1, 16));
    insertSpaces = settings.boolValue("Editor/insertSpaces", insertSpaces);
    restoreSession = settings.boolValue("Editor/restoreSession", restoreSession);
    findCaseSensitive = settings.boolValue("Editor/findCaseSensitive", findCaseSensitive);
    findWholeWords = settings.boolValue("Editor/findWholeWords", findWholeWords);
    findRegex = settings.boolValue("Editor/findRegex", findRegex);
    lastFolder = settings.stringValue("Editor/lastFolder", lastFolder);
}

std::string EditorSettings::defaultPath()
{
    return Lingmo::base::paths::lingmoConfig("text-editor");
}

void EditorSettings::commit()
{
    Settings settings(m_path);
    settings.setValue("Editor/fontFamily", fontFamily);
    settings.setValue("Editor/fontSize", fontSize);
    settings.setValue("Editor/wordWrap", wordWrap);
    settings.setValue("Editor/lineNumbers", lineNumbers);
    settings.setValue("Editor/highlightCurrentLine", highlightCurrentLine);
    settings.setValue("Editor/showWhitespace", showWhitespace);
    settings.setValue("Editor/matchBrackets", matchBrackets);
    settings.setValue("Editor/autoIndent", autoIndent);
    settings.setValue("Editor/tabWidth", tabWidth);
    settings.setValue("Editor/insertSpaces", insertSpaces);
    settings.setValue("Editor/restoreSession", restoreSession);
    settings.setValue("Editor/findCaseSensitive", findCaseSensitive);
    settings.setValue("Editor/findWholeWords", findWholeWords);
    settings.setValue("Editor/findRegex", findRegex);
    settings.setValue("Editor/lastFolder", lastFolder);
    settings.sync();
    changed();
}
