// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DQMLAPPPLUGINPRELOADINTERFACE_H
#define DQMLAPPPLUGINPRELOADINTERFACE_H

#include <QSGRendererInterface>
#include <dtkdeclarative_global.h>

#define DQmlAppPreloadInterface_iid "dtk.qml.app.preload.interface"

QT_BEGIN_NAMESPACE
class QGuiApplication;
class QQmlApplicationEngine;
class QQmlEngine;
QT_END_NAMESPACE

DQUICK_BEGIN_NAMESPACE

class DQmlAppPreloadInterface
{
public:
    DQmlAppPreloadInterface() = default;
    virtual ~DQmlAppPreloadInterface() = default;

    virtual QUrl preloadComponentPath() const = 0;
    virtual void aboutToPreload(QQmlApplicationEngine *engine);
    virtual QGuiApplication *creatApplication(int &argc, char **argv);
    virtual QSGRendererInterface::GraphicsApi graphicsApi();
};

DQUICK_END_NAMESPACE

Q_DECLARE_INTERFACE(DTK_QUICK_NAMESPACE::DQmlAppPreloadInterface, DQmlAppPreloadInterface_iid)
#endif // DQMLAPPPLUGINPRELOADINTERFACE_H
