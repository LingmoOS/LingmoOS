// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "maincomponentplugin.h"

#include <QUrl>

DQUICK_USE_NAMESPACE

MainComponentPlugin::MainComponentPlugin(QObject *parent)
    : QObject(parent)
{

}

MainComponentPlugin::~MainComponentPlugin()
{

}

QUrl MainComponentPlugin::mainComponentPath() const
{
    return QUrl("qrc:///apploader/main.qml");
}
