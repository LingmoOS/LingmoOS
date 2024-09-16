// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXHIBITIONWIDGET_P_H
#define EXHIBITIONWIDGET_P_H

#include "exhibitionwidget.h"

namespace GrandSearch {

class ExhibitionWidgetPrivate
{
public:
    explicit ExhibitionWidgetPrivate(ExhibitionWidget *parent = nullptr);

    ExhibitionWidget *q_p = nullptr;
};

}

#endif // EXHIBITIONWIDGET_P_H
