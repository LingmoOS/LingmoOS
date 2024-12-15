// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtklogin_global.h"

#include <qobject.h>

DLOGIN_BEGIN_NAMESPACE
class DLoginSeat;
class Login1SeatInterface;

class DLoginSeatPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DLoginSeatPrivate(DLoginSeat *parent = nullptr)
        : q_ptr(parent)
    {
    }

private:
    Login1SeatInterface *m_inter;
    DLoginSeat *q_ptr;
    Q_DECLARE_PUBLIC(DLoginSeat)
};

DLOGIN_END_NAMESPACE
