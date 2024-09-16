// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_stubpublic.h"

QList<QStringList>  LogApplicationHelper_getCustomLogList()
{
    return QList<QStringList>();
}

QList<QStringList>  LogApplicationHelper_getCustomLogList_002()
{
    QList<QStringList> list;
    list.append(QStringList()<<"test_name"<<"test_path");
    return list;
}

QAction *stub_menu_exec(const QPoint &pos, QAction *at)
{
    Q_UNUSED(pos);
    Q_UNUSED(at);
    return nullptr;
}

