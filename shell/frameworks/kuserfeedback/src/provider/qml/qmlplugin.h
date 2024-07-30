/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_QMLPLUGIN_H
#define KUSERFEEDBACK_QMLPLUGIN_H

#include <QQmlExtensionPlugin>

namespace KUserFeedback {

class QmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
    public:
        void registerTypes(const char *uri) override;
};

}

#endif // KUSERFEEDBACK_QMLPLUGIN_H
