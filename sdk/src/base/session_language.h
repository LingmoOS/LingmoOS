// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <memory>
#include <string>

#include "base/signal.h"

namespace Lingmo::base {

namespace dbus {
class Connection;
}

// Shorthand for the D-Bus namespace.
namespace dbus = Lingmo::base::dbus;

// Follows the session language that lingmo-services keeps: when it
// changes, the process's language variables and locale are set again and
// changed runs. ui::Application holds one; a process without it, such as a
// daemon, makes its own.
class SessionLanguage
{
public:
    explicit SessionLanguage(dbus::Connection* bus);
    ~SessionLanguage();

    SessionLanguage(const SessionLanguage&) = delete;
    SessionLanguage& operator=(const SessionLanguage&) = delete;

    Signal<> changed;

    // LANG, LANGUAGE and the LC_ variables, an empty value removing one; then
    // setlocale() and Locale::system() again. False when nothing differed.
    static bool apply(const std::map<std::string, std::string>& variables);

private:
    struct Private;
    std::unique_ptr<Private> d;
};

} // namespace Lingmo::base
