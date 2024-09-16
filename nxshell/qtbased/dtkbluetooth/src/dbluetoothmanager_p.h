// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHMANAGER_P_H
#define DBLUETOOTHMANAGER_P_H

#include "dagentmanagerinterface.h"
#include <DObjectPrivate>
#include "dbluetoothmanager.h"

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DObjectPrivate;

class DManagerPrivate : public DObjectPrivate
{
public:
    explicit DManagerPrivate(DManager *parent = nullptr);
    ~DManagerPrivate() override;

    DAgentManagerInterface *m_agentManager{nullptr};

    D_DECLARE_PUBLIC(DManager)
};

DBLUETOOTH_END_NAMESPACE

#endif
