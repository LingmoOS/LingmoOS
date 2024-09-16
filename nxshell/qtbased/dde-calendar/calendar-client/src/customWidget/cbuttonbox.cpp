// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cbuttonbox.h"

#include <QFocusEvent>

CButtonBox::CButtonBox(QWidget *parent)
    : DButtonBox(parent)
{
    //设置接受tab焦点切换
    this->setFocusPolicy(Qt::TabFocus);
}

void CButtonBox::focusInEvent(QFocusEvent *event)
{
    DButtonBox::focusInEvent(event);
    //窗口激活时，不设置Button焦点显示
    if (event->reason() != Qt::ActiveWindowFocusReason) {
        //设置当前选中项为焦点
        this->button(checkedId())->setFocus();
    }
}

void CButtonBox::focusOutEvent(QFocusEvent *event)
{
    DButtonBox::focusOutEvent(event);
    //当tab离开当前buttonbox窗口时，设置选中项为焦点
    if (event->reason() == Qt::TabFocusReason) {
        //设置当前选中项为焦点
        this->button(checkedId())->setFocus();
    }
}
