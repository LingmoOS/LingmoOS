// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "servicerunner.h"

#include <KIO/ApplicationLauncherJob>

void ServiceRunner::run()
{
    KIO::ApplicationLauncherJob(m_service).start();
}

QString ServiceRunner::name() const
{
    return m_name;
}

void ServiceRunner::setName(const QString &name)
{
    m_name = name;
    m_service = KService::serviceByDesktopName(name);
    Q_EMIT nameChanged();
    Q_EMIT canRunChanged();
}

bool ServiceRunner::canRun() const
{
    return !m_name.isEmpty() && m_service && m_service->isValid();
}

#include "moc_servicerunner.cpp"
