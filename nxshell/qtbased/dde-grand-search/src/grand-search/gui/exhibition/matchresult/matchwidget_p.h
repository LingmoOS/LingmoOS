// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MATCHWIDGET_P_H
#define MATCHWIDGET_P_H

#include "matchwidget.h"

namespace GrandSearch {

class MatchWidgetPrivate
{
public:
    explicit MatchWidgetPrivate(MatchWidget *parent = nullptr);
    void setGroupIcon(GroupWidget *wid);
    MatchWidget *q_p = nullptr;
};

}

#endif // MATCHWIDGET_P_H
