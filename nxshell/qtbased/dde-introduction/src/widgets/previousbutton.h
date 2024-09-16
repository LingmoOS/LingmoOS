// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIOUSBUTTON_H
#define PREVIOUSBUTTON_H

#include <DIconButton>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

class previousButton : public DIconButton
{
public:
    explicit previousButton(QWidget *parent = nullptr);

    void keyPressEvent(QKeyEvent *) override;
};

#endif // PREVIOUSBUTTON_H
