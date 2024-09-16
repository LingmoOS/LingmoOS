// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "maincomponentplugin.h"

#include <QVariant>

#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include <QProcessEnvironment>

#include "pluginlistmodel.h"
#include "staff/staff.h"

DQUICK_USE_NAMESPACE

MainComponentPlugin::MainComponentPlugin(QObject *parent)
    : QObject(parent)
{
    pluginListModel = new PluginListModel(this);
    registerStaff();
}

MainComponentPlugin::~MainComponentPlugin() {}

QUrl MainComponentPlugin::mainComponentPath() const
{
    // 返回程序的主控件部分 qml 文件，请确保该文件存在
    return QUrl("qrc:///main.qml");
}

void MainComponentPlugin::initialize(QQmlApplicationEngine *engine)
{
    engine->rootContext()->setContextProperty("pluginListModel", pluginListModel);
    engine->rootContext()->setContextProperty("env", QProcessEnvironment::systemEnvironment().toStringList());
}
