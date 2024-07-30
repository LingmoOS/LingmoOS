// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

#include "simulationctl.h"

#include <QFile>

static QString readCLIData()
{
    QFile file(QStringLiteral(":/plasma-disks/fixtures/cli.txt"));
    const bool open = file.open(QFile::ReadOnly);
    Q_ASSERT(open);
    return QString::fromLocal8Bit(file.readAll());
}

void SimulationCtl::run(const QString &devicePath)
{
    qDebug() << "SIMULATING" << devicePath;
    QFile file(devicePath);
    Q_ASSERT(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    Q_ASSERT(error.error == QJsonParseError::NoError);
    Q_EMIT finished(devicePath, document, readCLIData());
}

#include "moc_simulationctl.cpp"
