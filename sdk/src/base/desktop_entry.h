// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Lingmo::base {

struct DesktopAction
{
    std::string id;
    std::string name;
    std::string icon;
    std::string exec;
};

// A .desktop file of type Application, with its names in one language.
struct DesktopEntry
{
    std::string id;
    std::string path;
    std::string type;
    std::string name;
    std::string genericName;
    std::string comment;
    std::string icon;
    std::string exec;
    std::string tryExec;
    std::string workingDirectory;
    std::string startupWMClass;
    std::vector<std::string> categories;
    std::vector<std::string> keywords;
    std::vector<std::string> mimeTypes;
    std::vector<std::string> onlyShowIn;
    std::vector<std::string> notShowIn;
    std::vector<DesktopAction> actions;
    bool terminal = false;
    bool noDisplay = false;
    bool hidden = false;
    bool dbusActivatable = false;

    // The XDG display rules for this desktop: Hidden, NoDisplay, TryExec,
    // OnlyShowIn and NotShowIn.
    bool shouldShow() const;
    bool shouldShow(const std::vector<std::string>& desktops) const;

    // Exec split into a program and its arguments, field codes expanded with
    // the files or URLs given.
    std::vector<std::string> commandForArguments(const std::vector<std::string>& arguments) const;
    std::vector<std::string> command() const { return commandForArguments({}); }

    // The names of this desktop, as entries name it in OnlyShowIn.
    static std::vector<std::string> currentDesktops();
    // The user's first preferred language as a POSIX name, such as "zh_CN".
    static std::string displayLanguage();
    // An empty language is displayLanguage(). Nothing for a file that is
    // not an application, or has no name and is not hidden.
    static std::optional<DesktopEntry> parse(const std::string& id, const std::string& path,
                                             std::string_view contents, const std::string& language = {});
    // Exec split, without the field codes.
    static std::vector<std::string> parseExec(const std::string& exec);
};

} // namespace Lingmo::base
