// SPDX-FileCopyrightText: 2021 - 2023 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dconfigconn.h"
#include "helper.hpp"
#include "dconfigresource.h"

#include <DConfigFile>

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QFile>
#include <QDebug>

DCORE_USE_NAMESPACE

DSGConfigConn::DSGConfigConn(const ConnKey &key, QObject *parent)
    : QObject (parent),
      m_key(key)
{
}

DSGConfigConn::~DSGConfigConn()
{
}

ConnKey DSGConfigConn::key() const
{
    return m_key;
}

QString DSGConfigConn::path() const
{
    return formatDBusObjectPath(m_key);
}

bool DSGConfigConn::containsWithoutProp(const QString &key) const
{
    return keyList().contains(key);
}

void DSGConfigConn::setResource(DSGConfigResource *resource)
{
    m_resource = resource;
}

/*!
 \brief 返回配置内容的所有配置项
 \return
 */
QStringList DSGConfigConn::keyList() const
{
    return meta()->keyList();
}

/*!
 \brief 返回配置版本信息
 \return
 */
QString DSGConfigConn::version() const
{
    const auto ver = meta()->version();
    return QString("%1.%2").arg(ver.major).arg(ver.minor);
}

/*!
 \brief 返回指定配置项的描述信息
 \a key 配置项名称
 \a locale 语言版本,为空时返回默认语言的描述信息
 \return
 */
QString DSGConfigConn::description(const QString &key, const QString &locale)
{
    if (!contains(key))
        return QString();

    return meta()->description(key, locale.isEmpty() ? QLocale::AnyLanguage :  QLocale(locale));
}

/*!
 \brief 返回指定配置项的显示名称
 \a key 配置项名称
 \a locale 语言版本,为空时返回默认语言的显示信息
 \return
 */
QString DSGConfigConn::name(const QString &key, const QString &locale)
{
    if (!contains(key))
        return QString();

    return meta()->displayName(key, locale.isEmpty() ? QLocale::AnyLanguage :  QLocale(locale));
}

/*!
 \brief 释放资源引用
 当服务不使用此资源时,减少引用计数
 */
void DSGConfigConn::release()
{
    const QString &service = calledFromDBus() ? message().service() : "test.service";
    qCDebug(cfLog, "Received release request, service:%s, path:%s.", qPrintable(service), qPrintable(m_key));

    emit releaseChanged(service);
}

/*!
 \brief 设置指定配置项的值
 \a key 配置项名称
 \a value 需要设置的值
 */
void DSGConfigConn::setValue(const QString &key, const QDBusVariant &value)
{
    if (!contains(key))
        return;

    if (!hasPermissionByUid(key))
        return;

    const auto &v = decodeQDBusArgument(value.variant());
    qCDebug(cfLog) << "Set value, key:" << key << ", now value:" << v << ", old value:" << file()->value(key, cache());
    if(!file()->setValue(key, v, getAppid(), cache()))
        return;

    if (meta()->flags(key).testFlag(DConfigFile::Global)) {
        emit globalValueChanged(key);
    } else {
        emit valueChanged(key);
    }
}

void DSGConfigConn::reset(const QString &key)
{
    if (!contains(key))
        return;

    qCDebug(cfLog) << "Reset value, key:" << key << ", old value:" << file()->value(key, cache());
    if(!file()->setValue(key, QVariant(), getAppid(), cache()))
        return;

    if (meta()->flags(key).testFlag(DConfigFile::Global)) {
        emit globalValueChanged(key);
    } else {
        emit valueChanged(key);
    }
}

/*!
 \brief 返回指定配置项的值
 \a key 配置项名称
 \return
 */
QDBusVariant DSGConfigConn::value(const QString &key)
{
    if (!contains(key))
        return QDBusVariant();

    if (!hasPermissionByUid(key))
        return QDBusVariant();

    // Try to get value from cache.
    auto value = file()->cacheValue(cache(), key);
    if (value.isNull()) {
        const bool canFallback = m_resource->fallbackToGenericConfig();
        // Fallback to generic configuration.
        if (canFallback) {
            const auto uid = getConnectionKey(m_key);
            const auto &tmp = m_resource->noAppidFile()->cacheValue(m_resource->noAppidCache(uid), key);
            if (!tmp.isNull()) {
                value = tmp;
                qCDebug(cfLog) << "Get [" << key << "]'s cache value from generic configuration.";
            }
        }
        // Fallback to meta or global configuration.
        if (value.isNull())
            value = file()->value(key);

        // Fallback to generic meta configuration.
        if (value.isNull() && canFallback) {
            const auto &tmp = m_resource->noAppidFile()->value(key);
            if (!tmp.isNull()) {
                value = tmp;
                qCDebug(cfLog) << "Get [" << key << "]'s meta value from generic configuration.";
            }
        }
    }

    if (value.isNull()) {
        QString errorMsg = QString("[%1] Requires the value in [%2].").arg(key).arg(getAppid());
        qWarning() << qPrintable(errorMsg);
        if (calledFromDBus()) {
            sendErrorReply(QDBusError::Failed, errorMsg);
        }
        return QDBusVariant();
    }

    qCDebug(cfLog) << "Get value key:" << key << ", value:" << value;
    return QDBusVariant{value};
}

bool DSGConfigConn::isDefaultValue(const QString &key)
{
    if (!contains(key))
        return false;

    // Try to get value from cache.
    auto value = file()->cacheValue(cache(), key);
    const auto isDefault = !value.isValid();

    qCDebug(cfLog) << "Get isDefaultValue value key:" << key << ", isDefault:" << isDefault;
    return isDefault;
}

/*!
 \brief 返回指定配置项的可见性
 \a key 配置项名称
 \return
 */
QString DSGConfigConn::visibility(const QString &key)
{
    if (!contains(key))
        return QString();

    return meta()->visibility(key) == DTK_CORE_NAMESPACE::DConfigFile::Private ? QString("private") : QString("public");
}

QString DSGConfigConn::permissions(const QString &key)
{
    if (!contains(key))
        return QString();

    return meta()->permissions(key) == DTK_CORE_NAMESPACE::DConfigFile::ReadWrite ? QString("readwrite") : QString("readonly");
}

int DSGConfigConn::flags(const QString &key)
{
    return static_cast<int>(meta()->flags(key));
}

QString DSGConfigConn::getAppid() const
{
    if (m_appName.isEmpty()) {
        if (calledFromDBus()) {
            const QString &service = message().service();
            const_cast<DSGConfigConn *>(this)->m_appName = getProcessNameByPid(connection().interface()->servicePid(service));
        } else {
            const_cast<DSGConfigConn *>(this)->m_appName = QString("testappid");
        }
    }
    return m_appName;
}

bool DSGConfigConn::contains(const QString &key)
{
    if (containsWithoutProp(key))
        return true;

    QString errorMsg = QString("[%1] Requires Non-existent configure item [%2] in [%3].").arg(getAppid()).arg(key).arg(m_key);
    if (calledFromDBus())
        sendErrorReply(QDBusError::Failed, errorMsg);
    qWarning() << qPrintable(errorMsg);

    return false;
}

DConfigMeta *DSGConfigConn::meta() const
{
    return file()->meta();
}

DConfigFile *DSGConfigConn::file() const
{
    const auto &resourceKey = getResourceKey(m_key);
    return m_resource->getFile(resourceKey);
}

DConfigCache *DSGConfigConn::cache() const
{
    return m_resource->getCache(m_key);
}

bool DSGConfigConn::hasPermissionByUid(const QString &key) const
{
    auto flags = meta()->flags(key);
    if (flags.testFlag(DConfigFile::UserPublic))
        return true;

    if (!calledFromDBus())
        return true;

    const QString &service = message().service();
    const uint callerUid = connection().interface()->serviceUid(service);
    const auto connectionUid = getConnectionKey(m_key);
    bool hasPermission = callerUid == connectionUid;

    if (!hasPermission) {
        QString errorMsg = QString("[%1] No Permission configure item [%2] in [%3].").arg(getAppid()).arg(key).arg(m_key);
        sendErrorReply(QDBusError::AccessDenied, errorMsg);
        qWarning() << qPrintable(errorMsg);
    }
    return hasPermission;
}
