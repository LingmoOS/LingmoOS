/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "formats.h"
#include "kuserproxy.h"
#include <KAboutData>
#include <KFormat>

class KCoreAddonsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterSingletonType<Formats>(uri, 1, 0, "Format", [](QQmlEngine *, QJSEngine *) {
            return new Formats();
        });
        qRegisterMetaType<QLocale::FormatType>();
        qmlRegisterUncreatableMetaObject(KFormat::staticMetaObject, uri, 1, 0, "FormatTypes", QString());
        qmlRegisterType<KUserProxy>(uri, 1, 0, "KUser");
        qmlRegisterSingletonType(uri, 1, 0, "AboutData", [](QQmlEngine *engine, QJSEngine *) -> QJSValue {
            return engine->toScriptValue(KAboutData::applicationData());
        });
    }
};

#include "kcoreaddonsplugin.moc"
