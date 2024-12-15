// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dqmlapppreloadinterface.h"

#include <QGuiApplication>

DQUICK_BEGIN_NAMESPACE


void DQmlAppPreloadInterface::aboutToPreload(QQmlApplicationEngine *engine) {
    Q_UNUSED(engine);
}

QGuiApplication *DQmlAppPreloadInterface::creatApplication(int &argc, char **argv) {
    if (qGuiApp)
        return qGuiApp;
    return new QGuiApplication(argc, argv);
}

QSGRendererInterface::GraphicsApi DQmlAppPreloadInterface::graphicsApi() {
    return QSGRendererInterface::Unknown;  // auto select.
}

DQUICK_END_NAMESPACE
