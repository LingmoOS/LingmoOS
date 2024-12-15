// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSETTINGSMANAGER_P_H
#define DSETTINGSMANAGER_P_H

#include "dbus/dsettingsmanagerinterface.h"
#include "dsettingsmanager.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DSettingsManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DSettingsManagerPrivate(DSettingsManager *parent = nullptr);
    ~DSettingsManagerPrivate() override = default;

    DSettingsManager *q_ptr{nullptr};
    DSettingsManagerInterface *m_settings{nullptr};
    Q_DECLARE_PUBLIC(DSettingsManager)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
