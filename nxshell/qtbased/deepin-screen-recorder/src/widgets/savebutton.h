// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SAVEBUTTON_H
#define SAVEBUTTON_H

#include "toolbutton.h"

class SaveButton : public DPushButton
{
    Q_OBJECT
public:
    explicit SaveButton(DWidget *parent = 0);
    ~SaveButton();

signals:
    void saveAction();
    void expandSaveOption(bool expand);

private:
    ToolButton *m_saveBtn;
    ToolButton *m_listBtn;
};

#endif // SAVEBUTTON_H
