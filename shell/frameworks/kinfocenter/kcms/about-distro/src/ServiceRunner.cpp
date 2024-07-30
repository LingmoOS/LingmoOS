// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#include "ServiceRunner.h"

#include <KIO/ApplicationLauncherJob>

void ServiceRunner::run()
{
    KIO::ApplicationLauncherJob(m_service).start();
}

QString ServiceRunner::desktopFileName() const
{
    return m_desktopFileName;
}

void ServiceRunner::setDesktopFileName(const QString &name)
{
    m_desktopFileName = name;
    m_service = KService::serviceByDesktopName(name);
    Q_EMIT desktopFileNameChanged();
    Q_EMIT changed();
}

QString ServiceRunner::genericName() const
{
    if (m_service) {
        return m_service->genericName();
    }
    return {};
}

QString ServiceRunner::iconName() const
{
    if (m_service) {
        return m_service->icon();
    }
    return {};
}

bool ServiceRunner::canRun() const
{
    return !m_desktopFileName.isEmpty() && m_service && m_service->isValid();
}

#include "moc_ServiceRunner.cpp"
