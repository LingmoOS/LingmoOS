/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QColor>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "grabwidget.h"

class Utils : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE bool isValidColor(const QString &colorString) const
    {
        return QColor::isValidColor(colorString);
    }
};

class ColorPickerPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterType<GrabWidget>(uri, 2, 0, "GrabWidget");
        qmlRegisterSingletonType<Utils>(uri, 2, 0, "Utils", [](QQmlEngine *, QJSEngine *) {
            return new Utils();
        });
    }
};

#include "colorpickerplugin.moc"
