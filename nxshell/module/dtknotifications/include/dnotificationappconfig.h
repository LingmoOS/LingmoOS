// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <dexpected.h>

#include "dtknotification_global.h"

DNOTIFICATIONS_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

class DNotificationAppConfigPrivate;

class DNotificationAppConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appName READ appName)
    Q_PROPERTY(QString appIcon READ appIcon)
    Q_PROPERTY(bool notificationEnabled READ notificationEnabled WRITE setNotificationEnabled NOTIFY notificationEnabledChanged)
    Q_PROPERTY(bool previewEnabled READ previewEnabled WRITE setPreviewEnabled NOTIFY previewEnabledChanged)
    Q_PROPERTY(bool soundEnabled READ soundEnabled WRITE setSoundEnabled NOTIFY soundEnabledChanged)
    Q_PROPERTY(bool showInNotificationCenterEnabled READ showInNotificationCenterEnabled WRITE setShowInNotificationCenterEnabled NOTIFY showInNotificationCenterEnabledChanged)
    Q_PROPERTY(bool showInLockScreenEnabled READ showInLockScreenEnabled WRITE setShowInLockScreenEnabled NOTIFY showInLockScreenEnabledChanged)

public:
    explicit DNotificationAppConfig(const QString &id, QObject *parent = nullptr);
    ~DNotificationAppConfig();

    QString appName() const;
    QString appIcon() const;

    bool notificationEnabled() const;
    DExpected<void> setNotificationEnabled(bool enabled);

    // show message preview
    bool previewEnabled() const;
    DExpected<void> setPreviewEnabled(const bool enabled);

    // play a sound
    bool soundEnabled() const;
    DExpected<void> setSoundEnabled(const bool enabled);

    bool showInNotificationCenterEnabled() const;
    DExpected<void> setShowInNotificationCenterEnabled(const bool enabled);

    bool showInLockScreenEnabled() const;
    DExpected<void> setShowInLockScreenEnabled(const bool enabled);

Q_SIGNALS:
    void notificationEnabledChanged(const QString &id, const bool enabled);
    void previewEnabledChanged(const QString &id, const bool enabled);
    void soundEnabledChanged(const QString &id, const bool enabled);
    void showInNotificationCenterEnabledChanged(const QString &id, const bool enabled);
    void showInLockScreenEnabledChanged(const QString &id, const bool enabled);

private:
    QScopedPointer<DNotificationAppConfigPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DNotificationAppConfig)
};
typedef QSharedPointer<DNotificationAppConfig> DNotificationAppConfigPtr;

DNOTIFICATIONS_END_NAMESPACE
