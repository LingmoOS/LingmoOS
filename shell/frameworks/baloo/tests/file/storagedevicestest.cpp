/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCoreApplication>
#include <QDebug>

#include "storagedevices.h"

using namespace Baloo;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    StorageDevices sd;
    for (const auto& device : sd.allMedia()) {
        qDebug() << device.mountPath() << device.isUsable();
    }

    return 0;
}
