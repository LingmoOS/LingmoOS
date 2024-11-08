/*
    SPDX-FileCopyrightText: 2009 Alan Alpert <alan.alpert@nokia.com>
    SPDX-FileCopyrightText: 2010 Ménard Alexis <menard@kde.org>
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef COREBINDINGSPLUGIN_H
#define COREBINDINGSPLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QQmlPropertyMap>

#include <Lingmo/Lingmo>

#include "appletpopup.h"
#include "dialog.h"

struct TypesForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(Types)
    QML_UNCREATABLE("")
    QML_FOREIGN(Lingmo::Types)
};

struct PropertyMapForeign {
    Q_GADGET
    QML_ANONYMOUS
    QML_FOREIGN(QQmlPropertyMap)
};

struct AppletPopupForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(AppletPopup)
    QML_FOREIGN(LingmoQuick::AppletPopup)
};

struct PopupLingmoWindowForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(PopupLingmoWindow)
    QML_FOREIGN(LingmoQuick::PopupLingmoWindow)
};

struct DialogForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(Dialog)
    QML_FOREIGN(LingmoQuick::Dialog)
};

class CoreBindingsPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void initializeEngine(QQmlEngine *engine, const char *uri) override;
    void registerTypes(const char *uri) override;
};

#endif
