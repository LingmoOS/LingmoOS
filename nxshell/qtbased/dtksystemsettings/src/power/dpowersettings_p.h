// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dpowersettings.h"

DPOWER_BEGIN_NAMESPACE

class SystemPowerInterface;
class DaemonPowerInterface;

class DPowerSettingsPrivate : public QObject
{
    Q_OBJECT
public:
    DPowerSettingsPrivate(DPowerSettings *parent = nullptr)
        : q_ptr(parent){};

    virtual ~DPowerSettingsPrivate() { }

    void connectDBusSignal();

public:
    SystemPowerInterface *m_systemPowerInter;
    DaemonPowerInterface *m_daemonPowerInter;
    DPowerSettings *q_ptr;
    Q_DECLARE_PUBLIC(DPowerSettings)
};

DPOWER_END_NAMESPACE
