// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include "dtknotification_global.h"
#include "dabstractnotificationmodeconfig.h"

DNOTIFICATIONS_BEGIN_NAMESPACE

class DNotificationDNDModeConfigPrivate;
class DNotificationDNDModeConfig : public DAbstractNotificationModeConfig
{
    Q_OBJECT
    Q_PROPERTY(bool DNDModeInLockScreenEnabled READ DNDModeInLockScreenEnabled WRITE setDNDModeInLockScreenEnabled NOTIFY DNDModeInLockScreenEnabledChanged)
    Q_PROPERTY(bool openByTimeIntervalEnabled READ openByTimeIntervalEnabled WRITE setOpenByTimeIntervalEnabled NOTIFY openByTimeIntervalEnabledChanged)
    Q_PROPERTY(QString startTime READ startTime WRITE setStartTime NOTIFY startTimeChanged)
    Q_PROPERTY(QString endTime READ endTime WRITE setEndTime NOTIFY endTimeChanged)
    Q_PROPERTY(bool showIconEnabled READ showIconEnabled WRITE setShowIconEnabled NOTIFY showIconEnabledChanged)
    Q_DECLARE_PRIVATE(DNotificationDNDModeConfig)

public:
    DNotificationDNDModeConfig(const QString &name, QObject *parent = nullptr);
    ~DNotificationDNDModeConfig() override;

    bool enabled() const override;
    DExpected<void> setEnabled(bool enabled) override;

    bool DNDModeInLockScreenEnabled() const;
    DExpected<void> setDNDModeInLockScreenEnabled(bool enabled);

    bool openByTimeIntervalEnabled() const;
    DExpected<void> setOpenByTimeIntervalEnabled(bool enabled);

    QString startTime() const;
    DExpected<void> setStartTime(const QString &startTime);

    QString endTime() const;
    DExpected<void> setEndTime(const QString &endTime);

    bool showIconEnabled() const;
    DExpected<void> setShowIconEnabled(bool enabled);

Q_SIGNALS:
    void DNDModeInLockScreenEnabledChanged(const bool enabled);
    void openByTimeIntervalEnabledChanged(const bool enabled);
    void startTimeChanged(const QString& startTime);
    void endTimeChanged(const QString& endTime);
    void showIconEnabledChanged(const bool enabled);
};
typedef QSharedPointer<DNotificationDNDModeConfig> DNotificationDNDModeConfigPtr;

DNOTIFICATIONS_END_NAMESPACE
