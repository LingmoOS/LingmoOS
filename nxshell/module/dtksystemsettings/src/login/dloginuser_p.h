// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtklogin_global.h"

#include <qobject.h>

DLOGIN_BEGIN_NAMESPACE
class Login1UserInterface;
class DLoginUser;

class DLoginUserPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DLoginUserPrivate(DLoginUser *parent = nullptr)
        : q_ptr(parent)
    {
    }

private:
    Login1UserInterface *m_inter;
    DLoginUser *q_ptr;
    Q_DECLARE_PUBLIC(DLoginUser)
};

DLOGIN_END_NAMESPACE
