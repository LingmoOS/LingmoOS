// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "preloadplugin.h"

#include <QGuiApplication>
#include <QUrl>

DQUICK_USE_NAMESPACE

PreloadPlugin::PreloadPlugin(QObject *parent)
    : QObject(parent)
{

}

PreloadPlugin::~PreloadPlugin()
{

}

QUrl PreloadPlugin::preloadComponentPath() const
{
    return QUrl("qrc:///apploader/Preload.qml");
}
