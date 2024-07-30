/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef RBPURPOSEQUICKPLUGIN_H
#define RBPURPOSEQUICKPLUGIN_H

#include <QQmlExtensionPlugin>

class RBPurposeQuickPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // PURPOSEQUICKPLUGIN_H
