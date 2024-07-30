/*
 * SPDX-FileCopyrightText: 2015 Dominik Haumann <dhaumann@kde.org>
 * SPDX-FileCopyrightText: 2017 David Faure <faure@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "dict_object.h"
#include "dictionariesmodel.h"

// Qt
#include <QAbstractSocket>
#include <QWebEngineUrlScheme>

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class DictPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qRegisterMetaType<QAbstractSocket::SocketError>();

        qmlRegisterAnonymousType<QAbstractListModel>("", 1);

        qmlRegisterType<DictObject>(uri, 1, 0, "DictObject");
        qmlRegisterType<DictionariesModel>(uri, 1, 0, "DictionariesModel");

        QWebEngineUrlScheme::registerScheme(QWebEngineUrlScheme(QByteArrayLiteral("dict")));
    }
};

#include "dict_plugin.moc"
