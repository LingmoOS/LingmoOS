// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dconfig.h"
#ifndef D_DISABLE_DCONFIG
#include "dconfigfile.h"
#ifndef D_DISABLE_DBUS_CONFIG
#include "configmanager_interface.h"
#include "manager_interface.h"
#endif
#else
#include <QSettings>
#endif
#include "dobject_p.h"
#include <DSGApplication>

#include <QLoggingCategory>
#include <QCoreApplication>
#include <unistd.h>

// https://gitlabwh.uniontech.com/wuhan/se/deepin-specifications/-/issues/3

DCORE_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(cfLog)
static QString NoAppId;

/*!
@~english
    @class Dtk::Core::DConfigBackend
    \inmodule dtkcore

    @brief Configure the abstract interface of the backend.

    All configuration backends used by DConfig inherit this class, and users can inherit this class to implement their own configuration backends.
 */

/*!
@~english
    @fn bool DConfigBackend::load(const QString &) = 0

    @brief Initialize the backend

    \a appId Managed configuration information key value, the default is the application name.
  */

/*!
@~english
    @fn bool DConfigBackend::isValid() const = 0

    @sa DConfig::isValid().

 */

/*!
@~english
    @fn QStringList DConfigBackend::keyList() const = 0

    @sa DConfig::keyList()

 */

/*!
@~english
    @fn QVariant DConfigBackend::value(const QString &key, const QVariant &fallback = QVariant()) const = 0

    @sa DConfig::value()
 */

/*!
@~english
    @fn void DConfigBackend::setValue(const QString &key, const QVariant &value) = 0

    @sa DConfig::setValue()
 */

/*!
@~english
    @fn void DConfigBackend::reset(const QString &key)

    @sa DConfig::reset()
 */

/*!
@~english
    @fn QString DConfigBackend::name() const = 0

    @brief The unique identity of the backend configuration

/*!
@~english
 @fn bool DConfigBackend::isDefaultValue(const QString &key) const = 0

 @sa DConfig::isDefaultValue()

 */

DConfigBackend::~DConfigBackend()
{
}

static QString _globalAppId;
class Q_DECL_HIDDEN DConfigPrivate : public DObjectPrivate
{
public:
    explicit DConfigPrivate(DConfig *qq,
                            const QString &appId,
                            const QString &name,
                            const QString &subpath)
        : DObjectPrivate(qq)
        , appId(appId)
        , name(name)
        , subpath(subpath)
    {
    }

    virtual ~DConfigPrivate() override;

    inline bool invalid() const
    {
        const bool valid = backend && backend->isValid();
        if (!valid)
            qCWarning(cfLog, "DConfig is invalid of appid=%s name=%s, subpath=%s",
                      qPrintable(appId), qPrintable(name), qPrintable(subpath));

        return !valid;
    }

    DConfigBackend *getOrCreateBackend();
    DConfigBackend *createBackendByEnv();

    QString appId;
    QString name;
    QString subpath;
    QScopedPointer<DConfigBackend> backend;

    D_DECLARE_PUBLIC(DConfig)
};

namespace {

#ifndef D_DISABLE_DCONFIG
class Q_DECL_HIDDEN FileBackend : public DConfigBackend
{
public:
    explicit FileBackend(DConfigPrivate *o)
        : owner(o)
    {
    }

    virtual ~FileBackend() override;

    virtual bool isValid() const override
    {
        return configFile && configFile->isValid();
    }

    virtual bool load(const QString &/*appId*/) override
    {
        if (configFile)
            return true;

        configFile.reset(new DConfigFile(owner->appId,owner->name, owner->subpath));
        configCache.reset(configFile->createUserCache(getuid()));
        const QString &prefix = localPrefix();

        if (!configFile->load(prefix) || !configCache->load(prefix))
            return false;

        // generic config doesn't need to fallback to generic configration.
        if (owner->appId == NoAppId)
            return true;

        QScopedPointer<DConfigFile> file(new DConfigFile(NoAppId, owner->name, owner->subpath));
        const bool canFallbackToGeneric = !file->meta()->metaPath(prefix).isEmpty();
        if (canFallbackToGeneric) {
            QScopedPointer<DConfigCache> cache(file->createUserCache(getuid()));
            if (file->load(prefix) && cache->load(prefix)) {
                genericConfigFile.reset(file.take());
                genericConfigCache.reset(cache.take());
            }
        }
        return true;
    }

    virtual QStringList keyList() const override
    {
        return configFile->meta()->keyList();
    }

    virtual QVariant value(const QString &key, const QVariant &fallback) const override
    {
        const QVariant &vc = configFile->cacheValue(configCache.get(), key);
        if (vc.isValid())
            return vc;

        // fallback to generic configuration, and use itself's configuration if generic isn't set.
        if (genericConfigFile) {
            const auto &tmp = genericConfigFile->cacheValue(genericConfigCache.get(), key);
            if (tmp.isValid())
                return tmp;
        }
        const QVariant &v = configFile->value(key);
        if (v.isValid())
            return v;
        // fallback to default value of generic configuration.
        const QVariant &vg = genericConfigFile->value(key);
        return vg.isValid() ? vg : fallback;
    }

    virtual bool isDefaultValue(const QString &key) const override
    {
        // Don't fallback to generic configuration
        const QVariant &vc = configFile->cacheValue(configCache.get(), key);
        return !vc.isValid();
    }

    virtual void setValue(const QString &key, const QVariant &value) override
    {
        // setValue's callerAppid is itself instead of config's appId.
        if (configFile->setValue(key, value, DSGApplication::id(), configCache.get())) {
            Q_EMIT owner->q_func()->valueChanged(key);
        }
    }

    virtual void reset(const QString &key) override
    {
        setValue(key, QVariant());
    }

    virtual QString name() const override
    {
        return QString("FileBackend");
    }

private:
    QString localPrefix() const
    {
        if (!envLocalPrefix.isEmpty()) {
            return QString::fromLocal8Bit(envLocalPrefix);
        }
        return QString();
    }

private:
    QScopedPointer<DConfigFile> configFile;
    QScopedPointer<DConfigCache> configCache;
    QScopedPointer<DConfigFile> genericConfigFile;
    QScopedPointer<DConfigCache> genericConfigCache;
    DConfigPrivate* owner;
    const QByteArray envLocalPrefix = qgetenv("DSG_DCONFIG_FILE_BACKEND_LOCAL_PREFIX");
};

FileBackend::~FileBackend()
{
    const QString &prefix = localPrefix();
    if (configCache) {
        configCache->save(prefix);
        configCache.reset();
    }
    if (configFile) {
        configFile->save(prefix);
        configFile.reset();
    }
    if (genericConfigCache) {
        genericConfigCache->save(prefix);
        genericConfigCache.reset();
    }
    if (genericConfigFile) {
        genericConfigFile->save(prefix);
        genericConfigFile.reset();
    }
}

#ifndef D_DISABLE_DBUS_CONFIG

#define DSG_CONFIG "org.desktopspec.ConfigManager"
#define DSG_CONFIG_MANAGER "org.desktopspec.ConfigManager"

class Q_DECL_HIDDEN DBusBackend : public DConfigBackend
{
public:
    explicit DBusBackend(DConfigPrivate* o):
        owner(o)
    {
    }

    virtual ~DBusBackend() override;

    static bool isServiceRegistered()
    {
        return QDBusConnection::systemBus().interface()->isServiceRegistered(DSG_CONFIG);
    }

    static bool isServiceActivatable()
    {
         const QDBusReply<QStringList> activatableNames = QDBusConnection::systemBus().interface()->
                 callWithArgumentList(QDBus::AutoDetect,
                 QLatin1String("ListActivatableNames"),
                 QList<QVariant>());
//         qInfo() << activatableNames.value() << activatableNames.value().contains(DSG_CONFIG);

         return activatableNames.value().contains(DSG_CONFIG);
    }

    virtual bool isValid() const override
    {
        return config && config->isValid();
    }

    /*!
    @~english
      \internal

        Initialize the DBus connection, the call acquireManager dynamically obtains a configuration connection,
        The configuration file is then accessed through this configuration connection.
     */
    virtual bool load(const QString &/*appId*/) override
    {
        if (config)
            return true;

        qCDebug(cfLog, "Try acquire config manager object form DBus");
        DSGConfig dsg_config(DSG_CONFIG, "/", QDBusConnection::systemBus());
        QDBusPendingReply<QDBusObjectPath> dbus_reply = dsg_config.acquireManager(owner->appId, owner->name, owner->subpath);
        const QDBusObjectPath dbus_path = dbus_reply.value();
        if (dbus_reply.isError() || dbus_path.path().isEmpty()) {
            qCWarning(cfLog, "Can't acquire config manager. error:\"%s\"", qPrintable(dbus_reply.error().message()));
            return false;
        } else {
            qCDebug(cfLog(), "dbus path=\"%s\"", qPrintable(dbus_path.path()));
            config.reset(new DSGConfigManager(DSG_CONFIG_MANAGER, dbus_path.path(),
                                                QDBusConnection::systemBus(), owner->q_func()));
            if (!config->isValid()) {
                qCWarning(cfLog(), "Can't acquire config path=\"%s\"", qPrintable(dbus_path.path()));
                config.reset();
                return false;
            } else {
                QObject::connect(config.data(), &DSGConfigManager::valueChanged, owner->q_func(), &DConfig::valueChanged);
            }
        }
        return true;
    }

    virtual QStringList keyList() const override
    {
        return config->keyList();
    }

    static QVariant decodeQDBusArgument(const QVariant &v)
    {
        if (v.canConvert<QDBusArgument>()) {
            // we use QJsonValue to resolve all data type in DConfigInfo class, so it's type is equal QJsonValue::Type,
            // now we parse Map and Array type to QVariant explicitly.
            const QDBusArgument &complexType = v.value<QDBusArgument>();
            switch (complexType.currentType()) {
            case QDBusArgument::MapType: {
                QVariantMap list;
                complexType >> list;
                QVariantMap res;
                for (auto iter = list.begin(); iter != list.end(); iter++) {
                    res[iter.key()] = decodeQDBusArgument(iter.value());
                }
                return res;
            }
            case QDBusArgument::ArrayType: {
                QVariantList list;
                complexType >> list;
                QVariantList res;
                res.reserve(list.size());
                for (const auto &item : qAsConst(list)) {
                    res << decodeQDBusArgument(item);
                }
                return res;
            }
            default:
                qWarning("Can't parse the type, it maybe need user to do it, "
                         "QDBusArgument::ElementType: %d.", complexType.currentType());
            }
        }
        return v;
    }

    virtual QVariant value(const QString &key, const QVariant &fallback) const override
    {
        auto reply = config->value(key);
        reply.waitForFinished();
        if (reply.isError()) {
            qWarning() << "value error key:" << key << ", error message:" << reply.error().message();
            return fallback;
        }
        return decodeQDBusArgument(reply.value().variant());
    }

    virtual bool isDefaultValue(const QString &key) const override
    {
        auto reply = config->isDefaultValue(key);
        reply.waitForFinished();
        if (reply.isError()) {
            qWarning() << "Failed to call `isDefaultValue`, key:" << key
                       << ", error message:" << reply.error().message();
            return false;
        }
        return reply.value();
    }

    virtual void setValue(const QString &key, const QVariant &value) override
    {
        auto reply = config->setValue(key, QDBusVariant(value));
        reply.waitForFinished();
        if (reply.isError())
            qCWarning(cfLog) << "Failed to setValue for the key:" << key
                             << ", error message:" << reply.error();
    }

    virtual void reset(const QString &key) override
    {
        auto reply = config->reset(key);
        reply.waitForFinished();
        if (reply.isError())
            qCWarning(cfLog) << "Failed to reset for the key:" << key
                             << ", error message:" << reply.error();
    }

    virtual QString name() const override
    {
        return QString("DBusBackend");
    }

private:
    QScopedPointer<DSGConfigManager> config;
    DConfigPrivate* owner;
};

DBusBackend::~DBusBackend()
{
    if (config) {
        config->release();
    }
}
#endif //D_DISABLE_DBUS_CONFIG
#else

class Q_DECL_HIDDEN QSettingBackend : public DConfigBackend
{
public:
    explicit QSettingBackend(DConfigPrivate* o):
        owner(o)
    {
    }

    virtual ~QSettingBackend() override;

    virtual bool isValid() const override
    {
        return settings;
    }

    virtual bool load(const QString &appid) override
    {
        Q_UNUSED(appid);

        if (settings)
            return true;

        settings = new QSettings(owner->name, QSettings::IniFormat, owner->q_func());
        settings->beginGroup(owner->subpath);
        return true;
    }

    virtual QStringList keyList() const override
    {
        return settings->childKeys();
    }

    virtual QVariant value(const QString &key, const QVariant &fallback) const override
    {
        return settings->value(key, fallback);
    }

    virtual void setValue(const QString &key, const QVariant &value) override
    {
        settings->setValue(key, value);
    }

    virtual QString name() const override
    {
        return QString("QSettingBackend");
    }

private:
    QSettings *settings = nullptr;
    DConfigPrivate* owner;
};

QSettingBackend::~QSettingBackend()
{
}

#endif //D_DISABLE_DCONFIG
}

DConfigPrivate::~DConfigPrivate()
{
    backend.reset();
}

/*!
@~english
  \internal

    @brief Create a configuration backend

    The default configuration backend preferentially selects the D-Bus interface in the configuration center or the file configuration backend interface based on environment variables.
    If this environment variable is not configured, the configuration center service or file configuration backend interface will be selected according to whether the configuration center provides D-Bus services
 */
DConfigBackend *DConfigPrivate::getOrCreateBackend()
{
    if (backend) {
        return backend.data();
    }
    if (auto backendEnv = createBackendByEnv()) {
        backend.reset(backendEnv);
        return backend.data();
    }
#ifndef D_DISABLE_DCONFIG
#ifndef D_DISABLE_DBUS_CONFIG
    if (DBusBackend::isServiceRegistered() || DBusBackend::isServiceActivatable()) {
        qCDebug(cfLog, "Fallback to DBus mode");
        backend.reset(new DBusBackend(this));
    }
    if (!backend) {
        qCDebug(cfLog, "Can't use DBus config service, fallback to DConfigFile mode");
        backend.reset(new FileBackend(this));
    }
#else
    backend.reset(new FileBackend(this));
#endif //D_DISABLE_DBUS_CONFIG
#else
    qCDebug(cfLog, "Fallback to QSettings mode");
    backend.reset(new QSettingBackend(this));
#endif //D_DISABLE_DCONFIG
    return backend.data();
}

/*!
@~english
  \internal

    @brief Create a configuration backend

    Try to choose between configuring the D-Bus interface in the center or the file configuration backend interface based on the environment variables. 
 */
DConfigBackend *DConfigPrivate::createBackendByEnv()
{
    const QByteArray &envBackend = qgetenv("DSG_DCONFIG_BACKEND_TYPE");
    if (!envBackend.isEmpty()) {
        if (envBackend == "DBusBackend") {

#ifndef D_DISABLE_DCONFIG
#ifndef D_DISABLE_DBUS_CONFIG
            if (DBusBackend::isServiceRegistered() || DBusBackend::isServiceActivatable()) {
                qCDebug(cfLog, "Fallback to DBus mode");
                return new DBusBackend(this);
            }
#endif //D_DISABLE_DBUS_CONFIG
#endif //D_DISABLE_DCONFIG
        } else if (envBackend == "FileBackend") {

#ifndef D_DISABLE_DCONFIG
            qCDebug(cfLog, "Fallback to DConfigFile mode");
            return new FileBackend(this);
#endif //D_DISABLE_DCONFIG
        } else {

#ifndef D_DISABLE_DCONFIG
#else
            qCDebug(cfLog, "Fallback to QSettings mode");
            return new QSettingBackend(this);
#endif //D_DISABLE_DCONFIG
        }
    }
    return nullptr;
}

/*!
@~english
    @class Dtk::Core::DConfig
    \inmodule dtkcore

    @brief Configure the interface class provided by the policy

    
    This interface specification defines the relevant interfaces provided by the development library for reading and writing configuration files,
    If the application uses a development library that implements this specification, the application should use the interfaces provided by the development library first.
 */


/*!
@~english
 * @brief Constructs the objects provided by the configuration policy
 * \a name Configuration File Name
 * \a subpath Subdirectory corresponding to the configuration file
 * \a parent Parent object
 */
DConfig::DConfig(const QString &name, const QString &subpath, QObject *parent)
    : DConfig(nullptr, name, subpath, parent)
{
}

DConfig::DConfig(DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent)
    : DConfig(backend, _globalAppId.isEmpty() ? DSGApplication::id() : _globalAppId, name, subpath, parent)
{

}

/*!
@~english
 * @brief Constructs the object provided by the configuration policy, specifying the application Id to which the configuration belongs.
 * \a appId
 * \a name
 * \a subpath
 * \a parent
 * @return The constructed configuration policy object, which is released by the caller
 * @note \a appId is not empty.
 */
DConfig *DConfig::create(const QString &appId, const QString &name, const QString &subpath, QObject *parent)
{
    Q_ASSERT(appId != NoAppId);
    return new DConfig(nullptr, appId, name, subpath, parent);
}

DConfig *DConfig::create(DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent)
{
    Q_ASSERT(appId != NoAppId);
    return new DConfig(backend, appId, name, subpath, parent);
}

/*!
 * \brief Constructs the object, and which is application.
 * \param name
 * \param subpath
 * \param parent
 * \return Dconfig object, which is released by the caller
 * @note It's usually used for application independent, we should use DConfig::create if the configuration is a specific application.
 */
DConfig *DConfig::createGeneric(const QString &name, const QString &subpath, QObject *parent)
{
    return new DConfig(nullptr, NoAppId, name, subpath, parent);
}

DConfig *DConfig::createGeneric(DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent)
{
    return new DConfig(backend, NoAppId, name, subpath, parent);
}

/*!
 * \brief Explicitly specify application Id for config.
 * \param appId
 * @note It's should be called before QCoreApplication constructed.
 */
void DConfig::setAppId(const QString &appId)
{
    if (!_globalAppId.isEmpty()) {
        qCWarning(cfLog, "`setAppId`should only be called once.");
    }
    _globalAppId = appId;
    qCDebug(cfLog, "Explicitly specify application Id as appId=%s for config.", qPrintable(appId));
}

/*!
@~english
 * @brief Use custom configuration policy backend to construct objects
 * \a backend The caller inherits the configuration policy backend of DConfigBackend
 * \a appId The application Id of the configuration file. If it is blank, it will be the application Id by default
 * \a name Configuration File Name
 * \a subpath Subdirectory corresponding to the configuration file
 * \a parent Parent object
 * @note The caller only constructs backend, which is released by DConfig.
 */
DConfig::DConfig(DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent)
    : QObject(parent)
    , DObject(*new DConfigPrivate(this, appId, name, subpath))
{
    D_D(DConfig);

    qCDebug(cfLog, "Load config of appid=%s name=%s, subpath=%s",
            qPrintable(d->appId), qPrintable(d->name), qPrintable(d->subpath));

    if (backend) {
        d->backend.reset(backend);
    }

    if (auto backend = d->getOrCreateBackend()) {
        backend->load(d->appId);
    }
}

/*!
@~english
 * @brief DConfig::backendName
 * @return Configure policy backend name
 * @note The caller can only access the DConfigBackend object with DConfig, so the DConfigBackend object is not returned.
 */
QString DConfig::backendName() const
{
    D_DC(DConfig);
    if (d->invalid())
        return QString();

    return d->backend->name();
}

/*!
@~english
 * @brief Get all available configuration item names
 * @return Configuration item name collection
 */
QStringList DConfig::keyList() const
{
    D_DC(DConfig);
    if (d->invalid())
        return QStringList();

    return d->backend->keyList();
}

/*!
@~english
 * @brief Check whether the backend is available
 * @return
 */
bool DConfig::isValid() const
{
    D_DC(DConfig);
    return !d->invalid();
}

/*!
@~english
 * @brief Check whether the value is default according to the configuration item name
 * @param key Configuration Item Name
 * @return Return `true` if the value isn't been set, otherwise return `false`
 */
bool DConfig::isDefaultValue(const QString &key) const
{
    D_DC(DConfig);
    if (d->invalid())
        return false;
    return d->backend->isDefaultValue(key);
}

/*!
@~english
 * @brief Get the corresponding value according to the configuration item name
 * @param key Configuration Item Name
 * @param fallback The default value provided after the configuration item value is not obtained
 * @return
 */
QVariant DConfig::value(const QString &key, const QVariant &fallback) const
{
    D_DC(DConfig);
    if (d->invalid())
        return fallback;

    return d->backend->value(key, fallback);
}

/*!
@~english
 * @brief Set the value according to the configuration item name
 * @param key Configuration Item Name
 * @param value Values that need to be updated
 */
void DConfig::setValue(const QString &key, const QVariant &value)
{
    D_D(DConfig);
    if (d->invalid())
        return;

    d->backend->setValue(key, value);
}

/*!
@~english
 * @brief Set the default value corresponding to its configuration item. This value is overridden by the override mechanism. It is not necessarily the value defined in the meta in this configuration file
 * @param key Configuration Item Name
 */
void DConfig::reset(const QString &key)
{
    D_D(DConfig);
    if (d->invalid())
        return;

    d->backend->reset(key);
}

/*!
@~english
 * @brief Return configuration file name
 * @return
 */
QString DConfig::name() const
{
    D_DC(DConfig);
    return d->name;
}

/*!
@~english
 * @brief Return the subdirectory corresponding to the configuration file
 * @return
 */
QString DConfig::subpath() const
{
    D_DC(DConfig);
    return d->subpath;
}

DCORE_END_NAMESPACE
