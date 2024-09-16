// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STUBPUBLIC_H
#define STUBPUBLIC_H

#include <QDebug>
#include <QList>
#include <QStringList>
#include <QAction>


QList <QStringList>  LogApplicationHelper_getCustomLogList();
QList <QStringList>  LogApplicationHelper_getCustomLogList_002();
QAction *stub_menu_exec(const QPoint &pos, QAction *at = nullptr);

#endif // STUBPUBLIC_H
