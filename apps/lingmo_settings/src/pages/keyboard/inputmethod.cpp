// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/keyboard/inputmethod.h"

#include <cstdio>

#include "base/desktop_entry.h"
#include "base/file.h"
#include "base/paths.h"

using namespace Lingmo::base;

namespace settings {

// The method as its desktop entry names it, found where the user's entries
// override the system's.
static InputMethod::Method describe(const std::string& id)
{
    InputMethod::Method method{id, id, {}};
    std::vector<std::string> directories{paths::dataHome() + "/applications"};
    for (const std::string& directory : paths::dataDirectories())
        directories.push_back(directory + "/applications");
    for (const std::string& directory : directories) {
        const std::string path = directory + "/" + id;
        const std::optional<std::string> contents = file::read(path);
        if (!contents)
            continue;
        if (const std::optional<DesktopEntry> entry = DesktopEntry::parse(id, path, *contents)) {
            if (!entry->name.empty())
                method.name = entry->name;
            method.icon = entry->icon;
        }
        break;
    }
    return method;
}

InputMethod::InputMethod()
    : m_service("org.lingmo.InputMethod1")
{
    refreshMethods();
    m_wasAvailable = m_service.isAvailable();
    // The list is read again whenever the service comes back.
    m_serviceChanged = m_service.changed.connect([this] {
        if (m_service.isAvailable() != m_wasAvailable) {
            m_wasAvailable = m_service.isAvailable();
            refreshMethods();
        }
    });
}

void InputMethod::refreshMethods()
{
    m_methods.clear();
    if (!m_service.isAvailable())
        return;
    const dbus::Reply reply = m_service.proxy().call("availableInputMethods");
    for (const std::string& id : reply.value<std::vector<std::string>>().value_or(std::vector<std::string>()))
        m_methods.push_back(describe(id));
}

std::string InputMethod::current() const
{
    return m_service.stringValue("inputMethod");
}

bool InputMethod::isRunning() const
{
    return m_service.boolValue("running", false);
}

void InputMethod::select(const std::string& id)
{
    m_service.callWithReply("setInputMethod", [this, id](const dbus::Reply& reply) {
        if (reply.isError() || !reply.value<bool>().value_or(false)) {
            fprintf(stderr, "Cannot select input method %s: %s\n", id.c_str(), reply.error().message.c_str());
            refreshMethods();
        }
        m_service.changed.emit();
    }, id);
}

} // namespace settings
