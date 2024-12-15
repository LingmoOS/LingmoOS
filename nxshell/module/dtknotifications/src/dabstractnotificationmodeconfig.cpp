// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dabstractnotificationmodeconfig.h"
#include "dabstractnotificationmodeconfig_p.h"
#include <QDebug>

DNOTIFICATIONS_BEGIN_NAMESPACE

DAbstractNotificationModeConfigPrivate::DAbstractNotificationModeConfigPrivate(const QString &name, DAbstractNotificationModeConfig *q)
    : q_ptr(q),
      m_name(name)
{

}

DAbstractNotificationModeConfig::DAbstractNotificationModeConfig(const QString &name, QObject *parent)
    : QObject(parent),
      d_ptr(new DAbstractNotificationModeConfigPrivate(name, this))
{

}

DAbstractNotificationModeConfig::~DAbstractNotificationModeConfig()
{

}

QString DAbstractNotificationModeConfig::name() const
{
    Q_D(const DAbstractNotificationModeConfig);
    return d->m_name;
}

DAbstractNotificationModeConfig::DAbstractNotificationModeConfig(DAbstractNotificationModeConfigPrivate &d, QObject *parent)
    : d_ptr(&d)
{

}

DNOTIFICATIONS_END_NAMESPACE
