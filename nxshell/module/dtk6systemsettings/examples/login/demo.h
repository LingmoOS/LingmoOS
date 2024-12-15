// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include <qobject.h>

class LoginDemo : public QObject
{
public:
    LoginDemo(QObject *parent = nullptr);
    virtual int run() = 0;
};
