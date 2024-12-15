// SPDX-FileCopyrightText: 2021 - 2023 Uniontech Software Technology Co.,Ltd.
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
class DConfigMeta;
class DConfigCache;
DCORE_END_NAMESPACE

DCORE_USE_NAMESPACE
class DSGConfigConn;
class ConfigSyncRequestCache;
/**
 * @brief The DSGConfigResource class
 * 管理单个资源的所有链接和链接需要的配置功能，包括不同应用和应用间的配置
 */
class DSGConfigResource : public QObject
{
    Q_OBJECT
public:
    explicit DSGConfigResource(const QString &name, const QString &subpath, const QString &localPrefix = QString(), QObject *parent = nullptr);
    virtual ~DSGConfigResource() override;

    bool load(const QString &appid);

    GenericResourceKey key() const;
    DConfigFile *getFile(const ResourceKey &key) const;
    DConfigCache *getCache(const ConnKey &key) const;

    DSGConfigConn *getConn(const QString &appid, const uint uid) const;
    DSGConfigConn *getConn(const ConnKey &key) const;
    DSGConfigConn *createConn(const QString &appid, const uint uid);
    void removeConn(const ConnKey &connKey);
    bool isEmptyConn() const;
    ConnKey getConnKey(const QString &appid, const uint uid) const;
    int connSize() const;

    bool fallbackToGenericConfig() const;
    DConfigCache *noAppidCache(const uint uid) const;
    DConfigFile *noAppidFile() const;

    void save();
    void save(const QString &appid);

    bool reparse(const QString &appid);

    void setSyncRequestCache(ConfigSyncRequestCache *cache);
    void doSyncConfigCache(const ConfigCacheKey &key);

Q_SIGNALS:
    void releaseResource(const ConnServiceName &service);
    void releaseConn(const ConnServiceName &service, const ConnKey &connKey);
    void globalValueChanged(const QString &key);

private Q_SLOTS:
    void onValueChanged(const QString &key);
    void onGlobalValueChanged(const QString &key);
    void onReleaseChanged(const ConnServiceName &service);

private:
    void repareCache(DConfigCache *cache, DConfigMeta *oldMeta, DConfigMeta *newMeta);

    void doUpdateGenericConfigValueChanged(const QString &key, const ConnKey &connKey);

    void doGlobalValueChanged(const QString &key, const ResourceKey &resourceKey);

    DConfigFile *getOrCreateFile(const QString &appid);
    DConfigCache *createCache(const QString &appid, const uint uid);
    DConfigCache *getOrCreateCache(const QString &appid, const uint uid);
    QList<DSGConfigConn *> specificAppConns() const;
    bool cacheExist(const ResourceKey &key) const;
    QList<DConfigCache *> cachesOfTheResource(const ResourceKey &resourceKey) const;
    QList<DSGConfigConn *> connsOfTheResource(const ResourceKey &resourceKey) const;

private:
    GenericResourceKey m_key;
    QString m_fileName;
    QString m_subpath;
    QString m_localPrefix;

    QMap<ResourceKey, DConfigFile *> m_files;
    QMap<ConnKey, DConfigCache *> m_caches;
    QMap<ConnKey, DSGConfigConn *> m_conns;

    ConfigSyncRequestCache *m_syncRequestCache = nullptr;
};
