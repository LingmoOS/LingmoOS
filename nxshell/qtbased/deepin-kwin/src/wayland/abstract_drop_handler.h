/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "deepin-kwin_export.h"

#include "abstract_data_device.h"

namespace KWaylandServer
{
class SurfaceInterface;

class KWIN_EXPORT AbstractDropHandler : public AbstractDataDevice
{
    Q_OBJECT
public:
    AbstractDropHandler(QObject *parent = nullptr);
    virtual void updateDragTarget(SurfaceInterface *surface, quint32 serial) = 0;
    virtual void drop() = 0;
};
}
