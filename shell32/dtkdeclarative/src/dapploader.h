// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAPPLOADER_H
#define DAPPLOADER_H

#include <DObject>

#include <dtkdeclarative_global.h>
#include <QQmlComponent>

QT_BEGIN_NAMESPACE
class QString;
class QQmlEngine;
class QQuickWindow;
class QGuiApplication;
QT_END_NAMESPACE

DQUICK_BEGIN_NAMESPACE

class DAppLoaderPrivate;
class DAppLoader : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DAppLoader)
    D_PRIVATE_SLOT(void _q_onMainComponentStatusChanged(QQmlComponent::Status))
    D_PRIVATE_SLOT(void _q_onPreloadCreated(QObject *, const QUrl &))
    D_PRIVATE_SLOT(void _q_onComponentProgressChanged())

public:
    DAppLoader() = delete ;
    DAppLoader(const QString &appName, const QString &appPath = QString(), QObject *parent = nullptr);
    ~DAppLoader();

    void addPluginPath(const QString &dir);
    QStringList pluginPaths() const;

    int exec(int &argc, char **argv);
    static DAppLoader *instance() { return self; }

Q_SIGNALS:
    void loadFinished();
private:
    static DAppLoader *self;
};

DQUICK_END_NAMESPACE

#endif // DAPPLOADER_H
