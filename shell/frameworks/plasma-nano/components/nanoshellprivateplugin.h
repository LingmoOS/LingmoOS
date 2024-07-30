/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef NANOSHELLPRIVATEPLUGIN_H
#define NANOSHELLPRIVATEPLUGIN_H

#include <QQmlExtensionPlugin>

class PlasmaMiniShellPrivatePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif
