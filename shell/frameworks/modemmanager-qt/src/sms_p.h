/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Anant Kamath <kamathanant@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MODEMMANAGER_SMS_P_H
#define MODEMMANAGER_SMS_P_H

#include "dbus/smsinterface.h"
#include "sms.h"

#include <QDateTime>

namespace ModemManager
{
class SmsPrivate : public QObject
{
    Q_OBJECT
public:
    explicit SmsPrivate(const QString &path, Sms *q);
    OrgFreedesktopModemManager1SmsInterface smsIface;

    QString uni;
    MMSmsState state;
    MMSmsPduType pduType;
    QString number;
    QString text;
    QString smsc;
    QByteArray data;
    ValidityPair validity;
    int smsClass;
    bool deliveryReportRequest;
    uint messageReference;
    QDateTime timestamp;
    QDateTime dischargeTimestamp;
    MMSmsDeliveryState deliveryState;
    MMSmsStorage storage;
#if MM_CHECK_VERSION(1, 2, 0)
    MMSmsCdmaServiceCategory serviceCategory;
    MMSmsCdmaTeleserviceId teleserviceId;
#endif

    Q_DECLARE_PUBLIC(Sms)
    Sms *q_ptr;
private Q_SLOTS:
    void onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps);
};

} // namespace ModemManager

#endif
