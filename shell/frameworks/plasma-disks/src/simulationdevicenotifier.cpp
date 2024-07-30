// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

#include "simulationdevicenotifier.h"

#include <QDir>
#include <QDirIterator>

#include "device.h"

void SimulationDeviceNotifier::start()
{
    loadData();
}
void SimulationDeviceNotifier::loadData()
{
    QDirIterator it(QStringLiteral(":/plasma-disks/fixtures/"), {QStringLiteral("*.json")});
    while (it.hasNext()) {
        it.next();
        const auto info = it.fileInfo();
        Q_EMIT addDevice(new Device(info.fileName(), info.fileName(), info.absoluteFilePath()));
    }
}

#include "moc_simulationdevicenotifier.cpp"
