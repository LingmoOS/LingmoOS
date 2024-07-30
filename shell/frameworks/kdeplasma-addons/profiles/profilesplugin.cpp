/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "profilesmodel.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class ProfilesPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(uri == QByteArrayLiteral("org.kde.plasma.private.profiles"));

        qmlRegisterType<ProfilesModel>(uri, 1, 0, "ProfilesModel");
    }
};

#include "profilesplugin.moc"
