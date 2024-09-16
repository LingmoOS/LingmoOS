// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "demo.h"
#include "dloginmanager.h"
DLOGIN_USE_NAMESPACE

class PowerDemo : public LoginDemo
{
    Q_OBJECT
public:
    PowerDemo(QObject *parent = nullptr);
    int run() override;

private:
    DLoginManager *m_manager;
};
