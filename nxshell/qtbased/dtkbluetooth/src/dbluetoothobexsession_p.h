// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHOBEXSESSION_P_H
#define DBLUETOOTHOBEXSESSION_P_H

#include "dobexsessioninterface.h"
#include <DObjectPrivate>
#include "dbluetoothobexsession.h"
#include "dobexobjectpushinterface.h"

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DObjectPrivate;

class DObexSessionPrivate : public DObjectPrivate
{
public:
    explicit DObexSessionPrivate(const QString &path, DObexSession *parent = nullptr);
    ~DObexSessionPrivate() override;

    DObexSessionInterface *m_obexsession{nullptr};
    DObexObjectPushInterface *m_obexobjectpush{nullptr};
    D_DECLARE_PUBLIC(DObexSession)
};

DBLUETOOTH_END_NAMESPACE

#endif
