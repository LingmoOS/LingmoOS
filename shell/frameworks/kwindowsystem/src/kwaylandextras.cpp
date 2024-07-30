/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Aleix Pol <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kwaylandextras.h"

#include "kwindowsystem.h"
#include "kwindowsystem_p.h"

#include <QTimer>

KWaylandExtras::KWaylandExtras()
    : QObject()
{
}

KWaylandExtras::~KWaylandExtras() = default;

KWaylandExtras *KWaylandExtras::self()
{
    static KWaylandExtras instance;
    return &instance;
}

void KWaylandExtras::requestXdgActivationToken(QWindow *window, uint32_t serial, const QString &app_id)
{
    auto dv2 = dynamic_cast<KWindowSystemPrivateV2 *>(KWindowSystem::d_func());
    if (!dv2) {
        // Ensure that xdgActivationTokenArrived is always emitted asynchronously
        QTimer::singleShot(0, [serial] {
            Q_EMIT KWaylandExtras::self()->xdgActivationTokenArrived(serial, {});
        });

        return;
    }
    dv2->requestToken(window, serial, app_id);
}

quint32 KWaylandExtras::lastInputSerial(QWindow *window)
{
    auto dv2 = dynamic_cast<KWindowSystemPrivateV2 *>(KWindowSystem::d_func());
    if (!dv2) {
        return 0;
    }
    return dv2->lastInputSerial(window);
}

void KWaylandExtras::exportWindow(QWindow *window)
{
    if (auto dv2 = dynamic_cast<KWindowSystemPrivateV2 *>(KWindowSystem::d_func())) {
        dv2->exportWindow(window);
    }
}

void KWaylandExtras::unexportWindow(QWindow *window)
{
    if (auto dv2 = dynamic_cast<KWindowSystemPrivateV2 *>(KWindowSystem::d_func())) {
        dv2->unexportWindow(window);
    }
}

#include "moc_kwaylandextras.cpp"
