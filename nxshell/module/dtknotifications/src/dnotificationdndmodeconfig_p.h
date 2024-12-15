// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dnotificationdndmodeconfig.h"
#include "dabstractnotificationmodeconfig_p.h"

DNOTIFICATIONS_BEGIN_NAMESPACE

class DNotificationInterface;

class DNotificationDNDModeConfigPrivate : public DAbstractNotificationModeConfigPrivate
{
    Q_DECLARE_PUBLIC(DNotificationDNDModeConfig)
public:
    explicit DNotificationDNDModeConfigPrivate(const QString &name, DNotificationDNDModeConfig *q);
    ~DNotificationDNDModeConfigPrivate();

    DNotificationInterface *m_dNoticationInter;
};

DNOTIFICATIONS_END_NAMESPACE
