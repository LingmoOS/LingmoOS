// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGTABLEVIEW_H
#define LOGTABLEVIEW_H

#include <DTableView>

class LogTableView : public Dtk::Widget::DTableView
{
public:
    explicit LogTableView(QWidget *parent = nullptr);

    void focusInEvent(QFocusEvent *event);

    void focusOutEvent(QFocusEvent *event);
};

#endif  // LOGTABLEVIEW_H
