/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemsimple.h"

ModemSimple::ModemSimple(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

ModemSimple::~ModemSimple()
{
}

QDBusObjectPath ModemSimple::Connect(const QVariantMap &properties)
{
    Q_UNUSED(properties);
    // TODO
    return QDBusObjectPath();
}

void ModemSimple::Disconnect(const QDBusObjectPath &bearer)
{
    Q_UNUSED(bearer);
    // TODO
}

QVariantMap ModemSimple::GetStatus()
{
    // TODO
    return QVariantMap();
}

#include "moc_modemsimple.cpp"
