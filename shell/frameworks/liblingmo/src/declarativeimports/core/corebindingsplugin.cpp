/*
    SPDX-FileCopyrightText: 2009 Alan Alpert <alan.alpert@nokia.com>
    SPDX-FileCopyrightText: 2010 Ménard Alexis <menard@kde.org>
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "corebindingsplugin.h"

#include <QDebug>
#include <QQmlContext>
#include <QWindow>

#include <KLocalizedContext>

#include "action.h"

void CoreBindingsPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);

    QQmlContext *context = engine->rootContext();

    if (!context->contextObject()) {
        KLocalizedContext *localizedContextObject = new KLocalizedContext(engine);
        context->setContextObject(localizedContextObject);
    }
    // This ensures that importing lingmocore will make any KSvg use the current lasma theme
    new Lingmo::Theme(engine);
}

void CoreBindingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.kde.lingmo.core"));

    // HACK make properties like "opacity" work that are in REVISION 1 of QWindow
    qmlRegisterRevision<QWindow, 1>(uri, 2, 0);
    qmlRegisterRevision<QQuickItem, 1>(uri, 2, 0);
    qmlRegisterExtendedType<QAction, ActionExtension>(uri, 2, 0, "Action");
}

#include "moc_corebindingsplugin.cpp"
