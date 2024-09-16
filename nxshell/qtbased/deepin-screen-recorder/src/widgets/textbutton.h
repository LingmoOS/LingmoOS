// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTBUTTON_H
#define TEXTBUTTON_H

#include <DPushButton>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class TextButton : public DPushButton
{
    Q_OBJECT
public:
    TextButton(int num, DWidget *parent = 0);
    ~TextButton();

private:
    int m_fontsize;
};

#endif // TEXTBUTTON_H
