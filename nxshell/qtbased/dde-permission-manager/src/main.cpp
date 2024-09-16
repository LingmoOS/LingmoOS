// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DApplication>
#include <QTranslator>

#include "clientadaptor.h"
#include "permission1adaptor.h"

DWIDGET_USE_NAMESPACE

int main(int argc, char** argv)
{
    auto               app     = DApplication::globalApplication(argc, argv);

    QTranslator translator;
    qDebug() << "language:" << QLocale::system().name();
    translator.load("/usr/share/permission/translations/dde-permission-manager_" + QLocale::system().name());
    app->installTranslator(&translator);

    PermissionClient*  client  = new PermissionClient;
    PermissionService* service = new PermissionService;

    new ClientAdaptor(client);
    new Permission1Adaptor(service);

    QDBusConnection::sessionBus().registerService("org.deepin.dde.Permission1.Client");
    QDBusConnection::sessionBus().registerObject("/org/deepin.dde/Permission1/Client", client);

    QDBusConnection::sessionBus().registerService("org.deepin.dde.Permission1");
    QDBusConnection::sessionBus().registerObject("/org/deepin.dde/Permission1", service);

    app->setQuitOnLastWindowClosed(false);

    return app->exec();
}
