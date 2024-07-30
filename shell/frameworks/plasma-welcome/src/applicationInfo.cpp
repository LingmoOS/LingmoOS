/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "applicationInfo.h"

#include <KService>
#include <QString>

void ApplicationInfo::setDesktopName(const QString &desktopName)
{
    if (m_desktopName == desktopName) {
        return;
    }

    m_desktopName = desktopName;
    Q_EMIT desktopNameChanged();

    const KService::Ptr service = KService::serviceByDesktopName(m_desktopName);

    const bool exists = (service != nullptr);
    if (m_exists != exists) {
        m_exists = exists;
        Q_EMIT existsChanged();
    }

    if (!exists) {
        return;
    }

    const QString name = service->name();
    if (m_name != name) {
        m_name = name;
        Q_EMIT nameChanged();
    }

    const QString icon = service->icon();
    if (m_icon != icon) {
        m_icon = icon;
        Q_EMIT iconChanged();
    }
}

#include "moc_applicationInfo.cpp"
