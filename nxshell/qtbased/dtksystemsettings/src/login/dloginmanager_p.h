// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dloginmanager.h"

#include <qobject.h>
DLOGIN_BEGIN_NAMESPACE
class Login1ManagerInterface;

class DLoginManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DLoginManagerPrivate(DLoginManager *parent = nullptr)
        : QObject(parent)
        , q_ptr(parent)
    {
    }

public:
    Login1ManagerInterface *m_inter;
    DLoginManager *q_ptr;
    Q_DECLARE_PUBLIC(DLoginManager)
};

DLOGIN_END_NAMESPACE
