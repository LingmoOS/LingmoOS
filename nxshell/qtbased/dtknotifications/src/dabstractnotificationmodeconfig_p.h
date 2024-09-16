// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include "dtknotification_global.h"

DNOTIFICATIONS_BEGIN_NAMESPACE

class DAbstractNotificationModeConfig;

class DAbstractNotificationModeConfigPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DAbstractNotificationModeConfigPrivate(const QString &name, DAbstractNotificationModeConfig *q = nullptr);
    ~DAbstractNotificationModeConfigPrivate() = default;

    DAbstractNotificationModeConfig *q_ptr;
    QString m_name;
    Q_DECLARE_PUBLIC(DAbstractNotificationModeConfig)
};

DNOTIFICATIONS_END_NAMESPACE
