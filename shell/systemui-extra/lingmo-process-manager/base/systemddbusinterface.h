/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SYSTEMDDBUSINTERFACE_H
#define SYSTEMDDBUSINTERFACE_H

#include <QDBusVariant>
#include <QDBusConnection>
#include <string>

class SystemdDbusInterface
{
public:
    SystemdDbusInterface();
    std::string currentUserSliceName() const;
    std::string currentUserServiceName() const;
    std::string currentSessionScopeName() const;

    void createPersistentProcessGroup(const std::string &groupName);
    void createTransientProcessGroup(const std::string &name,
                                     std::vector<int> pids,
                                     const std::string &parentSlice);

    void setProcessGroupResourceLimit(
        const std::string &unitName, const std::string &attributeName, const std::string &value);

    void setSystemdUnitPropertyEnabled(
        const QString &unitName, const QString &propertyName, bool enabled);

    std::vector<int> getUnitProcessIds(const std::string &unitName) const;

private:
    using NamedVariant = QPair<QString, QDBusVariant>;
    using NamedVariantList = QList<NamedVariant>;
    void registerDbusTypes();

    std::string getCurrentLoginProperty(
        const std::string &property, const std::string &interface,
        const std::string &path) const;

    void setGeneralProperty(
        const std::string &unitName, const std::string &propertyName,
        const std::string &value);
    void setFreezerProperty(const std::string &unitName, const std::string &value);

    void freezeUnit(const std::string &unitName);
    void thawUnit(const std::string &unitName);

    void asyncCallSystemdDbus(
        const QString &method, const QList<QVariant> &arguments,
        QDBusConnection dbusConnection);
};

#endif // SYSTEMDDBUSINTERFACE_H
