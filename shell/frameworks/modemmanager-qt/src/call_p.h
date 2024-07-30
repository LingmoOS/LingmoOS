/*
    SPDX-FileCopyrightText: 2018 Aleksander Morgado <aleksander@aleksander.es>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGER_CALL_P_H
#define MODEMMANAGER_CALL_P_H

#include "call.h"
#include "dbus/callinterface.h"

namespace ModemManager
{
class CallPrivate : public QObject
{
    Q_OBJECT
public:
    explicit CallPrivate(const QString &path, Call *q);
    OrgFreedesktopModemManager1CallInterface callIface;

    QString uni;
    MMCallState state;
    MMCallStateReason stateReason;
    MMCallDirection direction;
    QString number;

    Q_DECLARE_PUBLIC(Call)
    Call *q_ptr;
private Q_SLOTS:
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps);
    void onStateChanged(int oldState, int newState, uint reason);
    void onDtmfReceived(const QString &dtmf);
};

} // namespace ModemManager

#endif
