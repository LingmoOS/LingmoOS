/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "cpuinfosource.h"

#include <QSysInfo>
#include <QThread>
#include <QVariant>

using namespace KUserFeedback;

CpuInfoSource::CpuInfoSource()
    : AbstractDataSource(QStringLiteral("cpu"), Provider::DetailedSystemInformation)
{
}

QString CpuInfoSource::description() const
{
    return tr("The amount and type of CPUs in the system.");
}

QVariant CpuInfoSource::data()
{
    QVariantMap m;
    m.insert(QStringLiteral("architecture"), QSysInfo::currentCpuArchitecture());
    m.insert(QStringLiteral("count"), QThread::idealThreadCount());
    return m;
}

QString CpuInfoSource::name() const
{
    return tr("CPU information");
}
