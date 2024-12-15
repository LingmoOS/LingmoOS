/*
    SPDX-FileCopyrightText: 2022 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/


#include "abstract_data_source.h"
#include "abstract_data_device.h"

namespace KWaylandServer
{

AbstractDataDevice::AbstractDataDevice(QObject *parent)
    : QObject(parent)
{
}

}
