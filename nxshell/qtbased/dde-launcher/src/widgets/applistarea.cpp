// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applistarea.h"

#include <QScrollBar>

AppListArea::AppListArea(QWidget *parent)
    : QScrollArea(parent)
{
}

void AppListArea::setHorizontalScrollValue(const int value)
{
    if (!horizontalScrollBar())
        return;

    horizontalScrollBar()->setValue(value);
}

int AppListArea::horizontalScrollValue() const
{
    if (!horizontalScrollBar())
        return 0;

    return horizontalScrollBar()->value();
}
