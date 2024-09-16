// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_common.h"

#include <DMenu>

#include <QProcess>

DWIDGET_USE_NAMESPACE

bool g_QWidget_isVisible_result = false;;               // QWidget isVisible返回值

UTCommon::UTCommon()
{

}

UTCommon::~UTCommon()
{

}

int qWidget_isVisible_stub()
{
    return g_QWidget_isVisible_result;
}

QAction *dmenu_exec_stub(const QPoint &, QAction *)
{
    return nullptr;
}

bool qProcess_startDetached_stub(const QString &, const QStringList &)
{
    return true;
}

void UTCommon::stub_DMenu_exec(Stub &stub)
{
    stub.set((QAction * (DMenu::*)(const QPoint &, QAction * at))ADDR(DMenu, exec), dmenu_exec_stub);
}

void UTCommon::stub_QWidget_isVisible(Stub &stub, bool isVisible)
{
    g_QWidget_isVisible_result = isVisible;
    stub.set(ADDR(QWidget, isVisible), qWidget_isVisible_stub);
}

void UTCommon::stub_QProcess_startDetached(Stub &stub)
{
#if !defined(Q_QDOC)
    stub.set((bool(*)(const QString &, const QStringList &))ADDR(QProcess, startDetached), qProcess_startDetached_stub);
#endif
}
