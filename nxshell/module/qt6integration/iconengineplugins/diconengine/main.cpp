/*
 * SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <qiconengineplugin.h>
#include <qstringlist.h>

#include <DIconTheme>

DGUI_USE_NAMESPACE

QT_BEGIN_NAMESPACE

class DIconEnginePlugin : public QIconEnginePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QIconEngineFactoryInterface" FILE "diconengine.json")

public:
    QStringList keys() const;
    QIconEngine *create(const QString &iconName = QString());
};

QStringList DIconEnginePlugin::keys() const
{
    return {"DIconProxyEngine"};
}

QIconEngine *DIconEnginePlugin::create(const QString &iconName)
{
    return DIconTheme::createIconEngine(iconName);
}

QT_END_NAMESPACE

#include "main.moc"
