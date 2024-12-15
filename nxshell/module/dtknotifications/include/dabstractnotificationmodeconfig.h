// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <dexpected.h>

#include "dtknotification_global.h"

DNOTIFICATIONS_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

class DAbstractNotificationModeConfigPrivate;

class DAbstractNotificationModeConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_DECLARE_PRIVATE(DAbstractNotificationModeConfig)

public:
    explicit DAbstractNotificationModeConfig(const QString &name, QObject *parent = nullptr);
    virtual ~DAbstractNotificationModeConfig() override;

    QString name() const;

    virtual bool enabled() const = 0;
    virtual DExpected<void> setEnabled(bool enabled) = 0;

Q_SIGNALS:
    void enabledChanged(const bool name);

protected:
    DAbstractNotificationModeConfig(DAbstractNotificationModeConfigPrivate &d, QObject *parent = nullptr);
    QScopedPointer<DAbstractNotificationModeConfigPrivate> d_ptr;
};
typedef QSharedPointer<DAbstractNotificationModeConfig> DAbstractNotificationModeConfigPtr;

DNOTIFICATIONS_END_NAMESPACE
