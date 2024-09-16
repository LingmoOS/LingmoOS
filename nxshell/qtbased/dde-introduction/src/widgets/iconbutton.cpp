// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconbutton.h"

IconButton::IconButton(QWidget *parent)
    : DFloatingButton(parent)
{
    setFixedSize(QSize(22,22));
    setIconSize(QSize(9,9));
    setIcon(DStyle::SP_MarkElement);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setFocusPolicy(Qt::NoFocus);
}

