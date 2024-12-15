// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABOUT_H
#define ABOUT_H

#include <QScrollArea>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class About : public QScrollArea
{
    Q_OBJECT
public:
    explicit About(QWidget *parent = nullptr);

};

#endif // ABOUT_H
