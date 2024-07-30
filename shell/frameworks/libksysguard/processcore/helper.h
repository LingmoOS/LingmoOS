/*
    KSysGuard, the KDE System Guard

    SPDX-FileCopyrightText: 2009 John Tapsell <john.tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later

*/

#ifndef KSYSGUARD_HELPER_H
#define KSYSGUARD_HELPER_H

#include <QObject>

#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>

using namespace KAuth;

/* The functions here run as ROOT.  So be careful. */

class KSysGuardProcessListHelper : public QObject
{
    Q_OBJECT
public:
    KSysGuardProcessListHelper();

public Q_SLOTS:
    ActionReply sendsignal(const QVariantMap &parameters);
    ActionReply renice(const QVariantMap &parameters);
    ActionReply changeioscheduler(const QVariantMap &parameters);
    ActionReply changecpuscheduler(const QVariantMap &parameters);
};

Q_DECLARE_METATYPE(QList<long long>)

#endif // KSYSGUARD_HELPER_H
