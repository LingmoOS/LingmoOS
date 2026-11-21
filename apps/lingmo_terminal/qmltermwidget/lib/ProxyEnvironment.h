// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROXYENVIRONMENT_H
#define PROXYENVIRONMENT_H

#include <map>
#include <string>
#include <vector>

// The session proxy as environment variables for a program started in the
// terminal, read from org.gnome.system.proxy in the GSettings keyfile, as
// lingmo-framework's ProxyEnvironment gives them.
namespace ProxyEnvironment
{

// Every variable the proxy owns; those the current mode does not set are dropped.
std::vector<std::string> names();
std::map<std::string, std::string> variables();

}

#endif // PROXYENVIRONMENT_H
