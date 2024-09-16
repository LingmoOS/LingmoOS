// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GROUPWIDGET_P_H
#define GROUPWIDGET_P_H

#include "groupwidget.h"

namespace GrandSearch {

class GroupWidgetPrivate
{
public:
    explicit GroupWidgetPrivate(GroupWidget *parent = nullptr);

    GroupWidget *q_p = nullptr;
};

}

#endif // GROUPWIDGET_P_H
