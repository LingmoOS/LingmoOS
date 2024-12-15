// SPDX-FileCopyrightText: 2021 - 2023 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dconfigresource.h"
#include "dconfigconn.h"
#include "dconfigrefmanager.h"
#include "dconfigfile.h"
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QFile>
#include <QDebug>

#include "manager_adaptor.h"

DCORE_USE_NAMESPACE

DSGConfigResource::DSGConfigResource(const QString &name, const QString &subpath, const QString &localPrefix, QObject *parent)
    : QObject (parent),
      m_key(getGenericResourceKey(name, subpath)),
      m_fileName(name),
      m_subpath(subpath),
      m_localPrefix(localPrefix)
{
}

DSGConfigResource::~DSGConfigResource()
{
    qDeleteAll(m_conns);
    m_conns.clear();

    save();

    qDeleteAll(m_files);
    m_files.clear();

    qDeleteAll(m_caches);
    m_caches.clear();
}

bool DSGConfigResource::load(const QString &appid)
{
    return getOrCreateFile(appid);
}

void DSGConfigResource::setSyncRequestCache(ConfigSyncRequestCache *cache)
{
    m_syncRequestCache = cache;
}

DSGConfigConn *DSGConfigResource::getConn(const QString &appid, const uint uid) const
{
    const ConnKey &connKey = getConnKey(appid, uid);
    return m_conns.value(connKey);
}

DSGConfigConn *DSGConfigResource::getConn(const ConnKey &key) const
{
    return m_conns.value(key);
}

DSGConfigConn *DSGConfigResource::createConn(const QString &appid, const uint uid)
{
    const ConnKey &connKey = getConnKey(appid, uid);

    DConfigCache *cache = m_caches.value(connKey);
    QScopedPointer<DConfigCache> cacheHolder;
    if (!cache) {
        cache = createCache(appid, uid);
        if (!cache) {
            qWarning() << QString("Create cache error for [%1]'s [%2].").arg(uid).arg(connKey);
            return nullptr;
        }

        cacheHolder.reset(cache);
    }

    QScopedPointer<DSGConfigConn> connPointer(new DSGConfigConn(connKey, this));
    if (qgetenv("DSG_CONFIG_CONNECTION_DISABLE_DBUS").isEmpty()) {
        (void) new DSGConfigManagerAdaptor(connPointer.get());
        QDBusConnection bus = QDBusConnection::systemBus();
        const auto path = connPointer->path();
        bus.unregisterObject(path);
        if (!bus.registerObject(path, connPointer.get())) {
            qWarning() << QString("Can't register the object %1.").arg(path);
            return nullptr;
        }
    }

    // Add cache to `m_caches` only initialized successful, otherwise it should be deleted.
    if (cacheHolder)
        m_caches.insert(connKey, cacheHolder.take());

    auto conn = connPointer.take();
    m_conns.insert(connKey, conn);
    conn->setResource(this);

    QObject::connect(conn, &DSGConfigConn::releaseChanged, this, &DSGConfigResource::onReleaseChanged);
    QObject::connect(conn, &DSGConfigConn::globalValueChanged, this, &DSGConfigResource::onGlobalValueChanged);
    QObject::connect(conn, &DSGConfigConn::valueChanged, this, &DSGConfigResource::onValueChanged);
    return conn;
}

ConnKey DSGConfigResource::getConnKey(const QString &appid, const uint uid) const
{
    return getConnectionKey(getResourceKey(appid, m_key), uid);
}

int DSGConfigResource::connSize() const
{
    return m_conns.size();
}

/*!
 \brief 重新解析文件
 \return 返回重新解析状态
 */
bool DSGConfigResource::reparse(const QString &appid)
{
    const auto &resouceKey = getResourceKey(appid, m_key);
    auto file = getFile(resouceKey);
    if (!file)
        return true;

    QScopedPointer<DConfigFile> config(new DConfigFile(*file));
    auto newMeta = config->meta();
    if (!newMeta->load()) {
        qWarning() << QString("Reparse resource error for [%1].").arg(m_key);
        return false;
    }

    QMap<DConfigCache*, QList<QString>> cacheChangedValues;
    DConfigMeta *oldMeta = file->meta();
    QList<DConfigCache*> caches;

    for (auto  item : cachesOfTheResource(resouceKey))
        caches.push_back(item);

    caches.push_back(file->globalCache());

    // cache and valuechanged.
    for (auto cache : caches) {
        QList<QString> changedValues;
        for (auto key : oldMeta->keyList()) {
            if (oldMeta->flags(key).testFlag(DConfigFile::Global) ^ cache->isGlobal())
                continue;

            if (file->value(key, cache) == config->value(key, cache))
                continue;

            changedValues.push_back(key);
        }
        if (!changedValues.isEmpty()) {
            cacheChangedValues[cache] = changedValues;
        }
        repareCache(cache, oldMeta, newMeta);
    }

    // config refresh.
    delete file;
    file = nullptr;
    m_files[resouceKey] = config.take();

    // emit valuechanged.
    for (auto iter = cacheChangedValues.begin(); iter != cacheChangedValues.end(); ++iter) {
        if (iter.key()->isGlobal()) {
            for (const QString &key : iter.value()) {
                doGlobalValueChanged(key, resouceKey);
            }
        } else {
            if (auto conn = getConn(appid, iter.key()->uid())) {
                for (const QString &key : iter.value()) {
                    emit conn->valueChanged(key);
                }
            } else {
                qWarning() << "Invalid connection:" << getConnKey(appid, iter.key()->uid());
            }
        }
    }
    qDebug(cfLog()) << "Those key's value changed:" << cacheChangedValues;

    return true;
}

void DSGConfigResource::onReleaseChanged(const ConnServiceName &service)
{
    auto conn = qobject_cast<DSGConfigConn*>(sender());
    if (conn) {
        emit releaseConn(service, conn->key());
    }
}

void DSGConfigResource::doSyncConfigCache(const ConfigCacheKey &key)
{
    if (ConfigSyncRequestCache::isUserKey(key)) {
        const auto connKey = ConfigSyncRequestCache::getUserKey(key);
        if (auto cache = getCache(connKey)) {
            qCDebug(cfLog()) << "Sync conn cache for user cache, key:" << key;
            cache->save(m_localPrefix);
        }
    } else if (ConfigSyncRequestCache::isGlobalKey(key)) {
        const auto resourceKey = ConfigSyncRequestCache::getGlobalKey(key);
        if (auto file = getFile(resourceKey)) {
            qCDebug(cfLog()) << "Sync conn cache for global cache, key:" << key;
            file->save(m_localPrefix);
        }
    } else {
        qCWarning(cfLog()) << "It's not exist config cache key" << key;
    }
}

bool DSGConfigResource::fallbackToGenericConfig() const
{
    // 判断是否需要fallback到公共配置
    DConfigFile file(NoAppId, m_fileName, m_subpath);
    const bool canFallbackToGeneric = !file.meta()->metaPath(m_localPrefix).isEmpty();
    return canFallbackToGeneric;
}

DConfigCache *DSGConfigResource::noAppidCache(const uint uid) const
{
    return const_cast<DSGConfigResource *>(this)->getOrCreateCache(VirtualInterAppId, uid);
}

DConfigFile *DSGConfigResource::noAppidFile() const
{
    return const_cast<DSGConfigResource *>(this)->getOrCreateFile(VirtualInterAppId);
}

DConfigFile *DSGConfigResource::getOrCreateFile(const QString &appid)
{
    const auto resourceKey = getResourceKey(appid, m_key);
    if (auto file = m_files.value(resourceKey))
        return file;

    QScopedPointer<DConfigFile> file(new DConfigFile(innerAppidToOuter(appid), m_fileName, m_subpath));
    file->globalCache()->setCachePathPrefix(DStandardPaths::path(DStandardPaths::XDG::ConfigHome) + "/global");
    if (!file->load(m_localPrefix))
        return nullptr;

    m_files.insert(resourceKey, file.data());
    return file.take();
}

DConfigCache *DSGConfigResource::getOrCreateCache(const QString &appid, const uint uid)
{
    const auto connKey = getConnectionKey(getResourceKey(appid, m_key), uid);
    if (auto cache = m_caches.value(connKey))
        return cache;

    if (auto cache = createCache(appid, uid)) {
        m_caches.insert(connKey, cache);
        return cache;
    }
    return nullptr;
}

DConfigCache *DSGConfigResource::createCache(const QString &appid, const uint uid)
{
    const auto resourceKey = getResourceKey(appid, m_key);
    if (auto file = getFile(resourceKey)) {
        QScopedPointer<DConfigCache> cache(file->createUserCache(uid));
        cache->setCachePathPrefix(DStandardPaths::path(DStandardPaths::XDG::ConfigHome) + QString("/%1").arg(uid));
        if (cache->load(m_localPrefix))
            return cache.take();
    }
    return nullptr;
}

DConfigFile *DSGConfigResource::getFile(const ResourceKey &key) const
{
    return m_files.value(key);
}

DConfigCache *DSGConfigResource::getCache(const ConnKey &key) const
{
    return m_caches.value(key);
}

/*
    @breaf Get all Connectons expect application independent Connection.
*/
QList<DSGConfigConn *> DSGConfigResource::specificAppConns() const
{
    QList<DSGConfigConn *> result;
    for (auto iter = m_conns.begin(); iter != m_conns.end(); ++iter) {
        if (isGenericResourceConn(iter.key()))
            continue;
        result << iter.value();
    }
    return result;
}

bool DSGConfigResource::cacheExist(const ResourceKey &key) const
{
    for (const auto &item : m_caches.keys()) {
        if (getResourceKey(item) == key)
            return true;
    }
    return false;
}

QList<DConfigCache *> DSGConfigResource::cachesOfTheResource(const ResourceKey &resourceKey) const
{
    QList<DConfigCache *> result;
    for (auto iter = m_caches.begin(); iter != m_caches.end(); ++iter) {
        if (getResourceKey(iter.key()) != resourceKey)
            continue;
        result << iter.value();
    }
    return result;
}

QList<DSGConfigConn *> DSGConfigResource::connsOfTheResource(const ResourceKey &resourceKey) const
{
    QList<DSGConfigConn *> result;
    for (auto iter = m_conns.begin(); iter != m_conns.end(); ++iter) {
        if (getResourceKey(iter.key()) != resourceKey)
            continue;
        result << iter.value();
    }
    return result;
}

void DSGConfigResource::onValueChanged(const QString &key)
{
    if (auto conn = qobject_cast<DSGConfigConn*>(sender())) {
        do {
            const auto &resouceKey = getResourceKey(conn->key());
            auto file = getFile(resouceKey);
            // global field changed don't cause user field to save, but `valueChanged` signal is emit.
            if (file && Q_UNLIKELY(file->meta()->flags(key).testFlag(DConfigFile::Global)))
                break;

            if (Q_LIKELY(m_syncRequestCache))
                m_syncRequestCache->pushRequest(ConfigSyncRequestCache::userKey(conn->key()));
        } while (false);

        // to emit generic configuration's valueChanged if valueChanged is emited from generic configuration resource.
        if (isGenericResourceConn(conn->key()))
            doUpdateGenericConfigValueChanged(key, conn->key());
    }
}

/*
  \internal

    \breaf update emit all app resource's valueChanged connected if noAppid configuration is changed.
*/
void DSGConfigResource::doUpdateGenericConfigValueChanged(const QString &key, const ConnKey &connKey)
{
//    only generic configuration resource to emit generic configuration's valueChanged.
    auto file = getFile(getResourceKey(connKey));
    if (!file)
        return;

    const bool isGlobal = file->meta()->flags(key).testFlag(DConfigFile::Global);
    const auto uid = getConnectionKey(connKey);
    for (auto conn : specificAppConns()) {
        if (!conn->containsWithoutProp(key))
            continue;

        if (isGlobal && uid == getConnectionKey(conn->key())) {
            doGlobalValueChanged(key, getResourceKey(conn->key()));
        } else {
            Q_EMIT conn->valueChanged(key);
        }
    }
}

void DSGConfigResource::doGlobalValueChanged(const QString &key, const ResourceKey &resourceKey)
{
    if (Q_LIKELY(m_syncRequestCache))
        m_syncRequestCache->pushRequest(ConfigSyncRequestCache::globalKey(resourceKey));
    // emit valueChanged of all conns for the resource.
    for (auto conn : connsOfTheResource(resourceKey)) {
        emit conn->valueChanged(key);
    }
}

/*
  \internal

    \breaf 重新解析缓存对象
*/
void DSGConfigResource::repareCache(DConfigCache *cache, DConfigMeta *oldMeta, DConfigMeta *newMeta)
{
    const auto newMetaKeys = newMeta->keyList();
    const auto oldMetaKeys = oldMeta->keyList();
    const QSet<QString> &newKeyList = {newMetaKeys.begin(), newMetaKeys.end()};
    const QSet<QString> &oldKeyList = {oldMetaKeys.begin(), oldMetaKeys.end()};

    // 配置项已经被移除，oldMeta - newMeta，移除cache值
    const auto subtractKeys = oldKeyList - (newKeyList);
    for (const auto &key :subtractKeys) {
        cache->remove(key);
        qDebug(cfLog, "Cache removed because of meta item removed, resource:%s, uid:%d, key:%s.",
               qPrintable(m_key), cache->uid(), qPrintable(key));
    }
    // 权限变化，ReadWrite -> ReadOnly，移除cache值
    auto intersectKeys = newKeyList;
    intersectKeys = intersectKeys.intersect(oldKeyList);
    for(const auto &key :intersectKeys) {
        if (newMeta->permissions(key) == DConfigFile::ReadOnly &&
                oldMeta->permissions(key) == DConfigFile::ReadWrite) {
            cache->remove(key);
            qDebug(cfLog, "Cache removed because of permissions changed from readwrite to readonly, resource:%s,uid:%d,key:%s.",
                   qPrintable(m_key), cache->uid(), qPrintable(key));
        }
    }
}

GenericResourceKey DSGConfigResource::key() const
{
    return m_key;
}

void DSGConfigResource::removeConn(const ConnKey &connKey)
{
    if (auto conn = getConn(connKey)) {
        m_conns.remove(connKey);
        conn->deleteLater();
    }

    if (auto cache = getCache(connKey)) {
        cache->save(m_localPrefix);
        m_caches.remove(connKey);
        delete cache;
    }

    const auto resourceKey = getResourceKey(connKey);
    if (auto file = getFile(resourceKey)) {
        if (!cacheExist(resourceKey)) {
            file->save(m_localPrefix);
            m_files.remove(resourceKey);
            delete file;
        }
    }

    qDebug(cfLog, "Removed connection:%s, remaining %d connection.", qPrintable(connKey), m_conns.count());
}

bool DSGConfigResource::isEmptyConn() const
{
    return m_conns.count() <= 0;
}

void DSGConfigResource::save()
{
    qDebug(cfLog, "Save resource's cache for [%s], and cache count:%d", qPrintable(m_key), m_caches.count());
    for (auto item : m_files)
        item->save(m_localPrefix);

    for (auto item : m_caches)
        item->save(m_localPrefix);
}

void DSGConfigResource::save(const QString &appid)
{
    const auto &resourceKey = getResourceKey(appid, m_key);
    if (auto file = getFile(resourceKey))
        file->save(m_localPrefix);

    for (auto item : cachesOfTheResource(resourceKey)) {
        item->save(m_localPrefix);
    }
}

void DSGConfigResource::onGlobalValueChanged(const QString &key)
{
    if (auto conn = qobject_cast<DSGConfigConn*>(sender())) {
        const auto &resourceKey = getResourceKey(conn->key());
        doGlobalValueChanged(key, resourceKey);
    }
}
