// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPACCESSIBLEBUTTON_H
#define APPACCESSIBLEBUTTON_H

#include "appaccessiblebase.h"

// 不要使用前置声明,被构造时需要做类型转换
#include <QPushButton>

class AppAccessibleButton : public AppAccessibleBase
{
public:
    explicit AppAccessibleButton(QPushButton *);
    virtual QString text(int startOffset, int endOffset) const Q_DECL_OVERRIDE;

private:
    QPushButton *m_widget;
};

#endif // APPACCESSIBLEBUTTON_H
