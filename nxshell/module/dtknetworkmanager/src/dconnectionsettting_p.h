// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCONNECTIONSETTTING_P_H
#define DCONNECTIONSETTTING_P_H

#include "dbus/dconnectionsettinginterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DConnectionSetting;

class DConnectionSettingPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DConnectionSettingPrivate(const quint64 id, DConnectionSetting *parent = nullptr);
    ~DConnectionSettingPrivate() override = default;

    DConnectionSetting *q_ptr{nullptr};
    DConnectionSettingInterface *m_setting{nullptr};
    Q_DECLARE_PUBLIC(DConnectionSetting)
};

DNETWORKMANAGER_END_NAMESPACE
#endif
