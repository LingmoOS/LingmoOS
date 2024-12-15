// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQMLAPPMAINWINDOWINTERFACE_H
#define DQMLAPPMAINWINDOWINTERFACE_H

#include <QGuiApplication>

#include <dtkdeclarative_global.h>

#define DQmlAppMainWindowInterface_iid "dtk.qml.app.mainwindow.interface"

QT_BEGIN_NAMESPACE
class QQmlApplicationEngine;
QT_END_NAMESPACE

DQUICK_BEGIN_NAMESPACE

class DQmlAppMainWindowInterface
{
public:
    DQmlAppMainWindowInterface() = default;
    virtual ~DQmlAppMainWindowInterface() = default;

    virtual QUrl mainComponentPath() const = 0;
    virtual void initialize(QQmlApplicationEngine *engine);
    virtual void finishedLoading(QQmlApplicationEngine *engine);
};

DQUICK_END_NAMESPACE

Q_DECLARE_INTERFACE(DTK_QUICK_NAMESPACE::DQmlAppMainWindowInterface, DQmlAppMainWindowInterface_iid)
#endif // DQMLAPPPLUGININTERFACE_H
