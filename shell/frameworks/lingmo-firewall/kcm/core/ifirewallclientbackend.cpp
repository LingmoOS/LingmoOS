// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
/*
 * UFW KControl Module
 */
#include <KLocalizedString>
#include <QProcess>

#include <QStandardPaths>

#include "ifirewallclientbackend.h"

IFirewallClientBackend::IFirewallClientBackend(QObject *parent, const QVariantList &)
    : QObject(parent){};

void IFirewallClientBackend::setProfiles(const QList<Entry> &profiles)
{
    m_profiles = profiles;
    std::sort(std::begin(m_profiles), std::end(m_profiles));
}

Entry IFirewallClientBackend::profileByName(const QString &name)
{
    auto it = std::find_if(std::begin(m_profiles), std::end(m_profiles), [&name](const Entry &entry) {
        return entry.name == name;
    });

    if (it != std::end(m_profiles)) {
        return *it;
    }

    return Entry({});
}

FirewallClient::Capabilities IFirewallClientBackend::capabilities() const
{
    return FirewallClient::None;
}

// just implement it when needed.
KJob *IFirewallClientBackend::save()
{
    return nullptr;
}

void IFirewallClientBackend::queryExecutable(const QString &executableName)
{
    // sometimes ufw is not installed on a standard path - like on opensuse, that's installed on /usr/sbin
    // so, look at there too.
    static QStringList searchPaths = {
        QStringLiteral("/bin"),
        QStringLiteral("/usr/bin"),
        QStringLiteral("/usr/sbin"),
    };

    m_executablePath = QStandardPaths::findExecutable(executableName);
    if (!m_executablePath.isEmpty()) {
        return;
    }
    m_executablePath = QStandardPaths::findExecutable(executableName, searchPaths);
}

bool IFirewallClientBackend::hasExecutable() const
{
    return !m_executablePath.isEmpty();
}

QString IFirewallClientBackend::executablePath() const
{
    return m_executablePath;
}
