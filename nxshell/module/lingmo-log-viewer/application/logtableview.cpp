// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logtableview.h"
#include <QDebug>

DWIDGET_USE_NAMESPACE

LogTableView::LogTableView(QWidget *parent)
    : DTableView(parent)
{
    this->setFocus();
}

void LogTableView::focusInEvent(QFocusEvent *event)
{
    qDebug() << "focus in";
}

void LogTableView::focusOutEvent(QFocusEvent *event)
{
    qDebug() << "focus in";
    //    this->setFocus();
    //    DTableView::focusOutEvent(event);
}
