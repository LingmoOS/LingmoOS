// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "global.h"
#include <QWindow>
#include <QGuiApplication>

QWindow * fromQtWinId(WId id) {
    QWindow *window = nullptr;

    for (auto win : qApp->allWindows()) {
        if (win->winId() == id) {
            window = win;
            break;
        }
    }
    return window;
};

DPP_BEGIN_NAMESPACE

RunInThreadProxy::RunInThreadProxy(QObject *parent)
    : QObject(parent)
{
}

void RunInThreadProxy::proxyCall(FunctionType func)
{
    QObject *receiver = parent();
    if (!receiver)
        receiver = qApp;

    QObject scope;
    connect(&scope, &QObject::destroyed, receiver, [func]() {
        (func)();
    }, Qt::QueuedConnection);
}

DPP_END_NAMESPACE
