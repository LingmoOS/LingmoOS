// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLUETOOTHADAPTER_P_H
#define DBLUETOOTHADAPTER_P_H

#include "dadapterinterface.h"
#include <DObjectPrivate>
#include "dbluetoothadapter.h"

DBLUETOOTH_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DObjectPrivate;

class DAdapterPrivate : public DObjectPrivate
{
public:
    explicit DAdapterPrivate(quint64 adapter, DAdapter *parent = nullptr);
    ~DAdapterPrivate() override;

    DAdapterInterface *m_adapter{nullptr};
    D_DECLARE_PUBLIC(DAdapter)
};

DBLUETOOTH_END_NAMESPACE

#endif
