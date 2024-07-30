/*
 *  SPDX-FileCopyrightText: 2009 Alan Alpert <alan.alpert@nokia.com>
 *  SPDX-FileCopyrightText: 2010 Ménard Alexis <menard@kde.org>
 *  SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef LINGMOUIPLUGIN_H
#define LINGMOUIPLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>
#include <QUrl>

class LingmoUIPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    LingmoUIPlugin(QObject *parent = nullptr);
    void registerTypes(const char *uri) override;
    void initializeEngine(QQmlEngine *engine, const char *uri) override;

#ifdef LINGMOUI_BUILD_TYPE_STATIC
    static LingmoUIPlugin &getInstance();
    static void registerTypes(QQmlEngine *engine = nullptr);
#endif

Q_SIGNALS:
    void languageChangeEvent();

private:
    QUrl componentUrl(const QString &fileName) const;
};

#endif
