// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QStringList>
#include <QDBusObjectPath>

#include <ddbusinterface.h>

using Dtk::Core::DDBusInterface;

class FakeDBusServiceParent: public QObject
{
    Q_OBJECT
public:
    explicit FakeDBusServiceParent(QObject *parent = nullptr);

    Q_PROPERTY(QList<QDBusObjectPath> objectPaths READ objectpaths NOTIFY objectpathsChanged);
    QList<QDBusObjectPath> objectpaths();
signals:
    void objectpathsChanged();
private:
    DDBusInterface *m_dDbusFakeDBusServiceInter;
};

class FakeDBusService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.FakeDBusService")
public:
    explicit FakeDBusService(QObject *parent = nullptr);
    ~FakeDBusService();

    Q_PROPERTY(QString strProperty READ strproperty WRITE setStrproperty)
    Q_PROPERTY(QList<QDBusObjectPath> objectPaths READ objectpaths)

    static QString get_service() { return "org.lingmo.FakeDBusService"; }
    static QString get_path() { return "/org/lingmo/FakeDBusService"; }
    static QString get_interface() { return "org.lingmo.FakeDBusService"; }

public Q_SLOTS:
    inline QString strproperty() { return m_strproperty; }
    void setStrproperty(const QString &var) {
        if (var != m_strproperty)
            m_strproperty = var;
    }
    inline QList<QDBusObjectPath> objectpaths() { return m_objectpaths; }
    // ExportScriptableContents
    Q_SCRIPTABLE QString foo() { return QString("bar"); }

private:
    const QString m_service{"org.lingmo.FakeDBusService"};
    const QString m_path{"/org/lingmo/FakeDBusService"};
    void registerService();
    void unregisterService();

    QString m_strproperty;
    QList<QDBusObjectPath> m_objectpaths;
};
