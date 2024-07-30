/*
 *  SPDX-FileCopyrightText: 2015 Lars Pontoppidan <dev.larpon@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mediaframe.h"

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class MediaFramePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterType<MediaFrame>(uri, 2, 0, "MediaFrame");
    }
};

#include "mediaframeplugin.moc"
