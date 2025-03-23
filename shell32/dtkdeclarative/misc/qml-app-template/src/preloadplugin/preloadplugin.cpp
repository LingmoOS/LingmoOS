// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "preloadplugin.h"

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
    // 预览组件的 qml 路径
    return QUrl("qrc:///Preload.qml");
}
