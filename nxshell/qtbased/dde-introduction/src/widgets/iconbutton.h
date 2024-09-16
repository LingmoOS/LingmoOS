// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <DFloatingButton>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

class IconButton : public DFloatingButton
{
public:
    explicit IconButton(QWidget *parent = nullptr);

};

#endif // ICONBUTTON_H
