// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtknotification_global.h"
#include <QObject>

DNOTIFICATIONS_BEGIN_NAMESPACE

class DNotificationManager;
class DNotificationInterface;

class DNotificationManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DNotificationManagerPrivate(DNotificationManager *parent = nullptr);
    ~DNotificationManagerPrivate() = default;

    DNotificationManager *q_ptr;
    DNotificationInterface *m_dNoticationInter;
    Q_DECLARE_PUBLIC(DNotificationManager)
};

DNOTIFICATIONS_END_NAMESPACE
