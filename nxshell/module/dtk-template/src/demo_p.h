// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef OCEANMO_P_H
#define OCEANMO_P_H

#include "demo.h"
#include "oceanmointerface.h"

OCEANMO_BEGIN_NAMESPACE

class DemoPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DemoPrivate(Demo *parent = nullptr);
    ~DemoPrivate() override = default;

    Demo *q_ptr{nullptr};
    DDemoInterface *m_inter{nullptr};
    Q_DECLARE_PUBLIC(Demo)
};

OCEANMO_END_NAMESPACE

#endif
