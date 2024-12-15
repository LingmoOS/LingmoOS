// SPDX-FileCopyrightText: 2021 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dconfig_global.h"
#include <dtkcore_global.h>
#include <QObject>
#include <QDBusObjectPath>
#include <QDBusContext>

DCORE_BEGIN_NAMESPACE
class DConfigFile;
class DConfigCache;
class DConfigMeta;
DCORE_END_NAMESPACE

/**
 * @brief The DSGConfigConn class
 * 管理单个链接
 * 配置文件的解析及方法调用
 */
class DSGConfigResource;
class DSGConfigConn : public QObject, protected QDBusContext
{
    Q_OBJECT
public:
    DSGConfigConn(const ConnKey &key, QObject *parent = nullptr);

    virtual ~DSGConfigConn() override;

    ConnKey key() const;
    QString path() const;
    bool containsWithoutProp(const QString &key) const;

    void setResource(DSGConfigResource *resource);
Q_SIGNALS:
    void releaseChanged(const ConnServiceName &service);

public: // PROPERTIES
    Q_PROPERTY(QStringList keyList READ keyList)
    QStringList keyList() const;

    Q_PROPERTY(QString version READ version)
    QString version() const;

public Q_SLOTS: // METHODS
    QString description(const QString &key, const QString &locale);
    QString name(const QString &key, const QString &locale);
    void release();
    void setValue(const QString &key, const QDBusVariant &value);
    void reset(const QString &key);
    QDBusVariant value(const QString &key);
    bool isDefaultValue(const QString &key);
    QString visibility(const QString &key) ;
    QString permissions(const QString &key) ;
    int flags(const QString &key);
Q_SIGNALS: // SIGNALS
    void valueChanged(const QString &key);
    void globalValueChanged(const QString &key);

private:
    QString getAppid() const;
    bool contains(const QString &key);
    DTK_CORE_NAMESPACE::DConfigMeta *meta() const;
    DTK_CORE_NAMESPACE::DConfigFile *file() const;
    DTK_CORE_NAMESPACE::DConfigCache *cache() const;
    bool hasPermissionByUid(const QString &key) const;

private:
    ConnKey m_key;
    DSGConfigResource *m_resource = nullptr;
    QString m_appName;
};

