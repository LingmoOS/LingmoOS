/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dummypowernotifier.h"

using namespace Solid;
DummyPowerNotifier::DummyPowerNotifier(QObject *parent)
    : PowerNotifier(parent)
{
    QMetaObject::invokeMethod(this, "emitEverythingChanged", Qt::QueuedConnection);
}

void DummyPowerNotifier::emitEverythingChanged()
{
    Q_EMIT acPluggedChanged(true);
}

#include "moc_dummypowernotifier.cpp"
