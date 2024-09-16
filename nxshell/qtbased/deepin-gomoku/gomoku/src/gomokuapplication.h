// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GOMOKUAPPLICATION_H
#define GOMOKUAPPLICATION_H

#include <DApplication>

DWIDGET_USE_NAMESPACE

class Gomokuapplication : public DApplication
{
public:
    explicit Gomokuapplication(int &argc, char **argv);

protected:
    void handleQuitAction() override; //重写菜单退出事件
};

#endif // GOMOKUAPPLICATION_H
