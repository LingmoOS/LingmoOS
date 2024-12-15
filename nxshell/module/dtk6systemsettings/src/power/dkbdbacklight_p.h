// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dkbdbacklight.h"

DPOWER_BEGIN_NAMESPACE

class UPowerKbdBacklightInterface;

class DKbdBacklightPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DKbdBacklightPrivate(DKbdBacklight *parent = nullptr)
        : QObject(parent)
        , q_ptr(parent)
    {
    }

    void connectDBusSignal();

public:
    UPowerKbdBacklightInterface *m_kb_inter;
    DKbdBacklight *q_ptr;
    Q_DECLARE_PUBLIC(DKbdBacklight)
};

DPOWER_END_NAMESPACE
