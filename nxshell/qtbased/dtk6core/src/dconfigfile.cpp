// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dconfigfile.h"

#include "dobject_p.h"
#include "filesystem/dstandardpaths.h"

#include <QFile>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QVariant>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QCollator>
#include <QDateTime>

#include <unistd.h>
#include <pwd.h>

// https://gitlabwh.uniontech.com/wuhan/se/deepin-specifications/-/issues/3

DCORE_BEGIN_NAMESPACE

#ifndef QT_DEBUG
Q_LOGGING_CATEGORY(cfLog, "dtk.dsg.config" , QtInfoMsg);
#else
Q_LOGGING_CATEGORY(cfLog, "dtk.dsg.config");
#endif

#define FILE_SUFFIX QLatin1String(".json")

// subpath must be a subdirectory of the dir.
inline static bool subpathIsValid(const QString &subpath, const QDir &dir)
{
    if (subpath.isEmpty())
        return true;

    const QDir subDir(dir.filePath(subpath.mid(1)));
    return subDir.absolutePath().startsWith(dir.absolutePath());
}
// name must be a valid filename.
inline static bool isValidFilename(const QString& filename)
{
    static const QRegularExpression regex("^[\\w\\-\\.\\ ]+$");
    QRegularExpressionMatch match = regex.match(filename);
    return match.hasMatch();
}
// AppId don't contain ' ', but it can be empty.
inline static bool isValidAppId(const QString& appId)
{
    static const QRegularExpression regex("^[\\w\\-\\.]*$");
    QRegularExpressionMatch match = regex.match(appId);
    return match.hasMatch();
}
/*!
@~english
  \internal

    @brief 按子目录查找机制查找配置文件

    在 \a baseDir目录下,查找名称为 \a name的文件,
    若存在 \a subpath,则从 \a subpath叶子目录逐级向上查找名称为 \a name的文件,
    若不存在此文件,则返回无效路径.
 */
inline QString getFile(const QString &baseDir, const QString &subpath, const QString &name,
                       bool canFallbackUp = true) {
    qCDebug(cfLog, "load json file from base dir:\"%s\", subpath = \"%s\", file name =\"%s\".",
            qPrintable(baseDir), qPrintable(subpath), qPrintable(name));

    const QDir base_dir(baseDir);
    if (!subpathIsValid(subpath, base_dir)) {
        qCDebug(cfLog, "subpath is invalid in the base dir:\"%s\", subpath:\"%s\".", qPrintable(baseDir), qPrintable(subpath));
        return QString();
    }
    QDir target_dir = base_dir;

    if (!subpath.isEmpty())
        target_dir.cd(subpath.mid(1));

    do {
        qCDebug(cfLog, "load json file from: \"%s\"", qPrintable(target_dir.path()));

        if (QFile::exists(target_dir.filePath(name))) {
            return target_dir.filePath(name);
        }

        if (base_dir == target_dir)
            break;
    } while (canFallbackUp && target_dir.cdUp());

    return QString();
}

inline QFile *loadFile(const QString &baseDir, const QString &subpath, const QString &name,
                       bool canFallbackUp = true)
{
    QString path = getFile(baseDir, subpath, name, canFallbackUp);
    if (!path.isEmpty()) {
        return new QFile(path);
    }
    return nullptr;
}

static QJsonDocument loadJsonFile(QIODevice *data)
{
    if (!data->open(QIODevice::ReadOnly)) {
        if (auto file = qobject_cast<QFile*>(data)) {
            qCDebug(cfLog, "Falied on open file: \"%s\", error message: \"%s\"",
                    qPrintable(file->fileName()), qPrintable(file->errorString()));
        }
        return QJsonDocument();
    }

    QJsonParseError error;
    auto document = QJsonDocument::fromJson(data->readAll(), &error);
    data->close();

    if (error.error != QJsonParseError::NoError) {
        qCWarning(cfLog, "%s", qPrintable(error.errorString()));
        return QJsonDocument();
    }

    return document;
}

static DConfigFile::Version parseVersion(const QJsonObject &obj) {
    DConfigFile::Version version {0, 0};
    const QString &verStr = obj[QLatin1String("version")].toString();

    if (verStr.isEmpty()) {
        return version;
    }

    const QStringList &items = verStr.split(QLatin1Char('.'));

    if (items.size() != 2)
        return version;

    bool ok = false;
    quint16 major = items.first().toUShort(&ok);

    if (!ok)
        return version;

    quint16 minor = items.last().toUShort(&ok);

    if (!ok)
        return version;

    version.major = major;
    version.minor = minor;

    return version;
}

#define MAGIC_META QLatin1String("dsg.config.meta")
#define MAGIC_OVERRIDE QLatin1String("dsg.config.override")
#define MAGIC_CACHE QLatin1String("dsg.config.cache")

static const uint InvalidUID = 0xFFFF;

inline static bool checkMagic(const QJsonObject &obj, QLatin1String request) {
    return obj[QLatin1String("magic")].toString() == request;
}

inline static bool versionIsValid(const DConfigFile::Version &v) {
    return v.major > 0 || v.minor > 0;
}

inline static bool checkVersion(const QJsonObject &obj, const DConfigFile::Version &request) {
    const DConfigFile::Version &v = parseVersion(obj);
    return versionIsValid(v) && v.major == request.major;
}

inline void overrideValue(QLatin1String subkey, const QJsonValue &from, QVariantHash &target) {
    const QJsonValue &v = from[subkey];

    if (!v.isUndefined())
        target[subkey] = v.toVariant();
}

inline static QString getUserName(const uint uid) {
    passwd *pw = getpwuid(uid);
    return pw ? QString::fromLocal8Bit(pw->pw_name) : QString();
}

/*!
@~english
    @class Dtk::Core::DConfigFile
    \inmodule dtkcore

    @brief Specification configuration file implementation of the interface that the configuration file reads and writes.
 */

/*!
@~english
    @enum DConfigFile::Flag

    \value NoOverride When this flag exists , it indicates that this configuration item can't be overwritten (see the override mechanism below for details).     
    Otherwise, the absence of this flag indicates that this configuration item is allowed to be overwritten,
    If it has a screen setting entry, hide or disable the screen setting entry when this entry is not writable.
    \value Global When reading or writing such a configuration, the user identity is ignored and the same data is obtained regardless of which user identity the program executes.
    The write operation will take effect for all users. However, if the corresponding configuration storage directory does not exist or has no write permission, this flag is ignored.
*/

/*!
@~english
    @enum DConfigFile::Permissions

    \value ReadOnly Overwrite the configuration item as readonly.
    \value ReadWrite Overwrite the configuration item as readable and writable.
*/

/*!
@~english
    @enum DConfigFile::Visibility

    \value Private For internal program use only,
    Not visible to the outside world. Such configuration items are completely read and written by the program itself, and can be added, deleted and rewritten at will without compatibility consideration,
    \value Public External programs are available.
    Once this type of configuration item is released, ensure that the configuration item is backward compatible during the upgrade of the compatible version.
    In short, this type of configuration item is only allowed to be deleted or modified when a major version of the program/library is upgraded.
    The configuration item is changed if any of its permissions, visibility, or flags properties are changed.
    In addition, you do not need to consider compatibility when modifying the value, name, and description attributes.
*/

/*!
@~english
    @struct Dtk::Core::DConfigFile::Version
    \inmodule dtkcore
    @brief Version Information

    The content format version of this file. The version number is described by two digits,
    Profiles with different first digits are incompatible with each other, and profiles with different second digits need to be compatible with each other.
    A program that reads this profile needs to perform content analysis by version, and when it encounters an incompatible version, it needs to terminate parsing immediately and ignore the file.
    And write a warning message to the program log, such as "1.0" and "2.0" versions are not compatible,
    If the parser only supports version 1.0, it should stop parsing when it encounters a 2.0 profile.
    But if version 1.1 is encountered, execution can continue.
    When writing to this profile, if an incompatible version is encountered, the current contents need to be cleared before writing, and this field needs to be updated with each write
*/

DConfigMeta::~DConfigMeta() {}

QStringList DConfigMeta::genericMetaDirs(const QString &localPrefix)
{
    QStringList paths;
    // lower priority is higher.
    for (auto item: DStandardPaths::paths(DStandardPaths::DSG::DataDir)) {
        paths.prepend(QDir::cleanPath(QString("%1/%2/configs").arg(localPrefix, item)));
    }
    return paths;
}

QStringList DConfigMeta::applicationMetaDirs(const QString &localPrefix, const QString &appId)
{
    QStringList paths;
    const auto &dataPaths = genericMetaDirs(localPrefix);
    paths.reserve(dataPaths.size());
    for (auto item : dataPaths) {
        paths << QString("%1/%2").arg(item, appId);
    }
    return paths;
}

Dtk::Core::DConfigCache::~DConfigCache() {}

struct DConfigKey {
    DConfigKey(const QString &aappId, const QString &afileName, const QString &asubpath)
        : appId(aappId),
          fileName(afileName),
          subpath(asubpath)
    {
    }

    explicit DConfigKey(const DConfigKey &src)
        : DConfigKey(src.appId, src.fileName, src.subpath)
    {
    }

    DConfigKey &operator = (const DConfigKey &src)
    {
        this->appId = src.appId;
        this->fileName = src.fileName;
        this->subpath = src.subpath;
        return *this;
    }

    QString appId;
    QString fileName;
    QString subpath;
};

class Q_DECL_HIDDEN DConfigInfo {
public:
    DConfigInfo()
    {

    }
    DConfigInfo(const DConfigInfo &other)
    {
        this->values = other.values;
    }
    DConfigInfo operator = (const DConfigInfo &other)
    {
        this->values = other.values;
        return *this;
    }
    inline static bool checkSerial(const int metaSerial, const int cacheSerial)
    {
        if (cacheSerial < 0)
            return true;
        if (metaSerial >= 0 && metaSerial == cacheSerial)
            return true;
        return false;
    }

    DConfigFile::Visibility visibility(const QString &key) const
    {
        DConfigFile::Visibility p = DConfigFile::Private;
        const auto &tmp = values[key][QLatin1String("visibility")].toString();
        if (tmp == QLatin1String("public"))
            p = DConfigFile::Public;

        return p;
    }

    DConfigFile::Permissions permissions(const QString &key) const
    {
        DConfigFile::Permissions p = DConfigFile::ReadOnly;
        const auto &tmp = values[key][QLatin1String("permissions")].toString();
        if (tmp == QLatin1String("readwrite"))
            p = DConfigFile::ReadWrite;

        return p;
    }

    DConfigFile::Flags flags(const QString &key) const
    {
        DConfigFile::Flags flags = {};
        const auto &tmp = values[key][QLatin1String("flags")];
        Q_FOREACH(const QString &flag, tmp.toStringList()) {
            if (flag == QLatin1String("nooverride")) {
                flags |= DConfigFile::NoOverride;
            } else if (flag == QLatin1String("global")) {
                flags |= DConfigFile::Global;
            } else if (flag == QLatin1String("user-public")) {
                flags |= DConfigFile::UserPublic;
            }
        }

        return flags;
    }

    QString displayName(const QString &key, const QLocale &locale) const
    {
        if (locale == QLocale::AnyLanguage)
            return values[key][QLatin1String("name")].toString();

        return values[key].value(QString("name[%1]")
                                 .arg(locale.name())).toString();
    }

    QString description(const QString &key, const QLocale &locale) const
    {
        if (locale == QLocale::AnyLanguage)
            return values[key][QLatin1String("description")].toString();

        return values[key].value(QString("description[%1]")
                                 .arg(locale.name())).toString();
    }

    inline QVariant value(const QString &key) const
    {
        return values[key][QLatin1String("value")];
    }

    inline int serial(const QString &key) const
    {
        bool status = false;
        const int tmp = values[key][QLatin1String("serial")].toInt(&status);
        if (status) {
            return tmp;
        }
        return -1;
    }

    inline void setValue(const QString &key, const QVariant &value)
    {
        values[key]["value"] = value;
    }

    inline void setSerial(const QString &key, const int &value)
    {
        values[key]["serial"] = value;
    }

    inline void setTime(const QString &key, const QString &value)
    {
        values[key]["time"] = value;
    }

    inline void setUser(const QString &key, const uint &value)
    {
        values[key]["user"] = getUserName(value);
    }

    inline void setAppId(const QString &key, const QString &value)
    {
        values[key]["appid"] = value;
    }

    inline QStringList keyList() const
    {
        return values.keys();
    }

    inline bool contains(const QString &key) const
    {
        return values.contains(key);
    }

    inline void remove(const QString &key)
    {
        values.remove(key);
    }

    inline bool update(const QString &key, const QVariantHash &value)
    {
        if (!value.contains("value")) {
            return false;
        }
        values[key] = value;
        return true;
    }

    inline bool updateValue(const QString &key, const QJsonValue &value)
    {
        return overrideValue(key, "value", value);
    }

    inline void updateSerial(const QString &key, const QJsonValue &value)
    {
        overrideValue(key, "serial", value);
    }

    inline void updatePermissions(const QString &key, const QJsonValue &value)
    {
        overrideValue(key, "permissions", value);
    }

    QJsonObject content() const
    {
        QJsonObject contents;
        for (auto i = values.constBegin(); i != values.constEnd(); ++i) {
            contents[i.key()] = QJsonObject::fromVariantHash(i.value());
        }
        return contents;
    }
private:
    bool overrideValue(const QString &key, const QString &subkey, const QJsonValue &from) {
        const QJsonValue &v = from[subkey];

        if (v.isUndefined()) {
            return false;
        }

        values[key][subkey] = v.toVariant();
        return true;
    }

    QHash<QString, QVariantHash> values;
};


/*!
@~english
    @class Dtk::Core::DConfigMeta
    \inmodule dtkcore

    @brief Provides a prototype of the configuration file and an access interface to the override mechanism.

*/

/*!
@~english
    @fn DConfigFile::Version DConfigMeta::version() const = 0;

    @brief Returns configuration version information.
    @return
*/

/*!
@~english
    @fn void DConfigMeta::setVersion(quint16 major, quint16 minor) = 0;

    @brief Sets configuration version information
    \a major Major version number
    \a minor Minor version number
*/

/*!
@~english
    @fn bool DConfigMeta::load(const QString &localPrefix = QString()) = 0;

    @brief Parsing configuration files
    \a localPrefix Directory prefix
    @return
*/

/*!
@~english
    @fn bool DConfigMeta::load(QIODevice *meta, const QList<QIODevice*> &overrides) = 0;

    @brief Parse the configuration file stream
    \a meta Prototype stream
    \a overrides The file stream to find for the override mechanism
    @return
*/

/*!
@~english
    @fn QStringList DConfigMeta::keyList() const = 0;

    @brief Returns all configuration items for the configuration content
    @return
*/

/*!
@~english
    @fn DConfigFile::Flags DConfigMeta::flags(const QString &key) const = 0;

    @brief Returns the attribute of the specified configuration item
    \a key Configure the name of the option, NoOverride This option can't be overridden, and Global ignores the user identity
    @return
*/

/*!
@~english
    @fn DConfigFile::Permissions DConfigMeta::permissions(const QString &key) const = 0;

    @brief Returns the permission for the specified configuration item
    \a key Configuration name
    @return

*/

/*!
@~english
    @fn DConfigFile::Visibility DConfigMeta::visibility(const QString &key) const = 0;

    @brief Returns the visibility of the specified configuration item
    \a key Configuration name
    @return

*/

/*!
@~english
    @fn int DConfigMeta::serial(const QString &key) const = 0;

    @brief Returns a monotonically increasing value of a configuration item
    \a key Configuration name
    @return An invalid value of -1 indicates that the entry is not configured

*/

/*!
@~english
    @fn QString DConfigMeta::displayName(const QString &key, const QLocale &locale) = 0;

    @brief Returns the display name of the specified configuration
    \a key Configuration name
    \a locale Language version
    @return
*/

/*!
@~english
    @fn QString DConfigMeta::description(const QString &key, const QLocale &locale) = 0;

    @brief Returns a description of the specified configuration item
    \a key Configuration name
    \a locale Language version
    @return

*/

/*!
@~english
    @fn QString DConfigMeta::metaPath(const QString &localPrefix = QString(), bool *useAppId = nullptr) const = 0;

    @brief Returns the path to the profile
    \a localPrefix Directory of all override mechanisms that need to be searched
    @return
*/

/*!
@~english
    @fn QStringList DConfigMeta::allOverrideDirs(const bool useAppId, const QString &prefix = QString()) const = 0;

    @brief Gets all the override mechanism directories to look for for the \a prefix directory
    \a useAppId Whether not to use the generic directory
    @return
*/

/*!
@~english
    @fn QVariant DConfigMeta::value(const QString &key) const = 0;

    @brief Original value of meta overwritten by the overwriting mechanism
    \a key Configuration name
    @return
*/

class Q_DECL_HIDDEN DConfigMetaImpl : public DConfigMeta {
    // DConfigMeta interface
public:
    explicit DConfigMetaImpl(const DConfigKey &configKey);
    virtual ~DConfigMetaImpl() override;

    inline virtual QStringList keyList() const override
    {
        return values.keyList();
    }
    inline virtual DConfigFile::Flags flags(const QString &key) const override
    {
        return values.flags(key);
    }
    inline virtual DConfigFile::Permissions permissions(const QString &key) const override
    {
        return values.permissions(key);
    }
    inline virtual DConfigFile::Visibility visibility(const QString &key) const override
    {
        return values.visibility(key);
    }
    inline virtual int serial(const QString &key) const override
    {
        return values.serial(key);
    }
    inline virtual QString description(const QString &key, const QLocale &locale) override
    {
        return values.description(key, locale);
    }
    virtual DConfigFile::Version version() const override
    {
        return m_version;
    }
    inline virtual void setVersion(quint16 major, quint16 minor) override
    {
        m_version.major = major;
        m_version.minor = minor;
    }
    inline virtual QString displayName(const QString &key, const QLocale &locale) override
    {
        return values.displayName(key, locale);
    }
    inline virtual QVariant value(const QString &key) const override
    {
        return values.value(key);
    }

    QString metaPath(const QString &localPrefix, bool *useAppId) const override
    {
        bool useAppIdForOverride = true;

        QString path;
        const QStringList &applicationMetas = applicationMetaDirs(localPrefix, configKey.appId);
        for (auto iter = applicationMetas.rbegin(); iter != applicationMetas.rend(); iter++) {
            path = getFile(*iter, configKey.subpath, configKey.fileName + FILE_SUFFIX);
            if (!path.isEmpty())
                break;
        }

        if (path.isEmpty()) {
            useAppIdForOverride = false;
            const QStringList &genericnMetas = genericMetaDirs(localPrefix);
            for (auto iter = genericnMetas.rbegin(); iter != genericnMetas.rend(); iter++) {
                path = getFile(*iter, configKey.subpath, configKey.fileName + FILE_SUFFIX);
                if (!path.isEmpty())
                    break;
            }
        }
        if (useAppId) {
            *useAppId = useAppIdForOverride;
        }
        return path;
    }

    bool load(const QString &localPrefix) override
    {
        if (!isValidAppId(configKey.appId)) {
            qCWarning(cfLog, "AppId is invalid, appId=%s", qPrintable(configKey.appId));
            return false;
        }
        if (!isValidFilename(configKey.fileName)) {
            qCWarning(cfLog, "Name is invalid, filename=%s", qPrintable(configKey.fileName));
            return false;
        }
        bool useAppIdForOverride = true;
        QString path = metaPath(localPrefix, &useAppIdForOverride);
        if (path.isEmpty()) {
            qCWarning(cfLog, "Can't load meta file from local prefix: \"%s\"", qPrintable(localPrefix));
            return false;
        }

        QScopedPointer<QFile> meta(new QFile(path));

        struct _ScopedPointer {
            explicit _ScopedPointer(const QList<QIODevice*> &list)
                : m_list(list) {}
            ~_ScopedPointer() {qDeleteAll(m_list);}

            QList<QIODevice*> m_list;
        };
        _ScopedPointer overrides(loadOverrides(localPrefix, useAppIdForOverride));

        return load(meta.data(), overrides.m_list);
    }

    bool load(QIODevice *meta, const QList<QIODevice*> &overrides) override
    {
        {
            const QJsonDocument &doc = loadJsonFile(meta);
            if (!doc.isObject())
                return false;

            // 检查标识
            const QJsonObject &root = doc.object();
            if (!checkMagic(root, MAGIC_META)) {
                qCWarning(cfLog, "The meta magic does not match");
                return false;
            }

            // 检查版本兼容性
            const auto &v = parseVersion(root);
            if (!versionIsValid(v) || v.major > DConfigFile::supportedVersion().major) {
                qCWarning(cfLog, "The meta version number does not match, "
                                 "the file major version=%i, supported major version<=%i",
                          v.major, DConfigFile::supportedVersion().major);
                return false;
            }

            m_version = v;

            const auto &contents = root[QLatin1String("contents")].toObject();
            auto i = contents.constBegin();

            // 初始化原始值
            for (; i != contents.constEnd(); ++i) {
                if (!values.update(i.key(), i.value().toObject().toVariantHash())) {
                    qWarning() << "key:" << i.key() << "has no value";
                    return false;
                }
            }
        }
        // for override
        Q_FOREACH(auto override, overrides) {
            const QJsonDocument &doc = loadJsonFile(override);
            if (doc.isObject()) {
                const QJsonObject &root = doc.object();
                if (!checkMagic(root, MAGIC_OVERRIDE)) {
                    if (auto file = static_cast<QFile*>(override)) {
                        qCWarning(cfLog, "The override magic does not match, file: \"%s\", error message: \"%s\"",
                                  qPrintable(file->fileName()), qPrintable(file->errorString()));
                    } else {
                        qCWarning(cfLog, "The override magic does not match");
                    }
                    break; //TODO don't continue parse?
                }
                if (!checkVersion(root, m_version)) {
                    qCWarning(cfLog, "The override version number does not match");
                    break;
                }

                if (auto file = static_cast<QFile*>(override)) {
                    qCDebug(cfLog, "The override will be applied, file: \"%s\"", qPrintable(file->fileName()));
                }

                const auto &contents = root[QLatin1String("contents")].toObject();
                auto i = contents.constBegin();

                for (; i != contents.constEnd(); ++i) {
                    if (!values.contains(i.key())) {
                        qCWarning(cfLog, "The meta doesn't contain the override key: \"%s\".", qPrintable(i.key()));
                        continue;
                    }
                    // 检查是否允许 override
                    if (values.flags(i.key()) & DConfigFile::NoOverride)
                        continue;

                    if (!values.updateValue(i.key(), i.value())) {
                        qWarning() << "key (override):" << i.key() << "has no value";
                        return false;
                    }
                    values.updateSerial(i.key(), i.value());
                    values.updatePermissions(i.key(), i.value());
                }
            }
        }

        return true;
    }
    /*!
    @~english
      \internal

        @brief 获得前缀为 \a prefix 目录的应用或公共库的所有覆盖机制目录,越后优先级越高
     */
    inline QStringList overrideDirs(const QString & prefix, bool useAppId) const {
        const QString &path2 = QString("%1/etc/dsg/configs/overrides/%2/%3")
                .arg(prefix, useAppId ? configKey.appId : QString(), configKey.fileName);

        QStringList paths;
        const QStringList &dataPaths = DStandardPaths::paths(DStandardPaths::DSG::DataDir);
        paths.reserve(dataPaths.size() + 1);
        for (auto path: dataPaths) {
            // reverse `DataDir`'s paths, previous `DataDir`'s value has high priority
            paths.prepend(QString("%1%2/configs/overrides/%3/%4")
                          .arg(prefix, path, useAppId ? configKey.appId : QString(), configKey.fileName));
        }
        // 在后面的优先级更高
        paths.append(path2);
        return paths;
    }

    inline QStringList allOverrideDirs(const bool useAppId, const QString &prefix) const override
    {
        QStringList dirs;
        // 只有当允许不使用 appid 时才能回退到通用目录
        if (!useAppId) {
            dirs << overrideDirs(prefix, false);
        }
        // 无论如何都先从带 appid 的目录下加载override文件
        // 在列表后面的更优先
        dirs << overrideDirs(prefix, true);
        return dirs;
    }
    /*!
    @~english
      \internal

        @brief 获得所有遵守覆盖机制的文件流

        在override文件放置路径下按优先级查找覆盖文件,支持子目录查找机制,
        使用自然排序（如“a2”在“a11”之前）规则按文件名进行排序
     */
    QList<QIODevice *> loadOverrides(const QString &prefix, bool useAppId) const
    {
        auto filters = QDir::Files | QDir::NoDotAndDotDot | QDir::Readable;
        const QStringList nameFilters {"*" + FILE_SUFFIX};

        QStringList dirs = allOverrideDirs(useAppId, prefix);

        QList<QIODevice*> list;
        list.reserve(50);
        QCollator collator(QLocale::English);
        collator.setNumericMode(true);
        collator.setIgnorePunctuation(true);

        Q_FOREACH(const auto &dir, dirs) {
            const QDir base_dir(QDir::cleanPath(dir));

            if (!base_dir.exists())
                continue;

            if (!subpathIsValid(configKey.subpath, base_dir))
                continue;

            QDir target_dir = base_dir;
            target_dir.setFilter(filters);
            target_dir.setNameFilters(nameFilters);

            if (!configKey.subpath.isEmpty())
                target_dir.cd(configKey.subpath.mid(1));

            do {
                qCDebug(cfLog, "load override file from: \"%s\"", qPrintable(target_dir.path()));

                QDirIterator iterator(target_dir);
                QList<QIODevice*> sublist;
                sublist.reserve(50);
                while(iterator.hasNext()) {
                    sublist.append(new QFile(iterator.next()));
                }

                // 从小到大排序
                std::sort(sublist.begin(), sublist.end(), [&collator](QIODevice *f1, QIODevice *f2){
                    if (collator.compare(static_cast<const QFile*>(f1)->fileName(),
                                         static_cast<const QFile*>(f2)->fileName()) < 0)
                        return true;
                    return false;
                });

                list = sublist + list;

                if (base_dir.path() == target_dir.path())
                    break;
            } while (target_dir.cdUp());
        }

        return list;
    }

    DConfigKey configKey;
    DConfigInfo values;
    DConfigFile::Version m_version = {0, 0};
    char padding [4] = {};
};

DConfigMetaImpl::DConfigMetaImpl(const DConfigKey &configKey)
    : DConfigMeta (),
     configKey(configKey)
{
}

DConfigMetaImpl::~DConfigMetaImpl()
{
}

/*!
@~english
    @class Dtk::Core::DConfigCache
    \inmodule dtkcore

    @brief Provides user and global runtime cache access interfaces for Configuration file.

*/

/*!
@~english
    @fn bool DConfigCache::load(const QString &localPrefix = QString()) = 0;
    @brief Parse the cache configuration file
    @return
*/

/*!
@~english
    @fn bool DConfigCache::save(const QString &localPrefix = QString(), QJsonDocument::JsonFormat format = QJsonDocument::Indented, bool sync = false) = 0;
    @brief Save the cached value to disk
    \a localPrefix Directory prefix
    \a format Save format
    \a sync Whether to refresh immediately
    @return
*/

/*!
@~english
    @fn bool DConfigCache::isGlobal() const = 0;
    @brief Whether to cache globally
    @return
*/

/*!
@~english
    @fn void DConfigCache::remove(const QString &key) = 0;
    @brief Delete a configuration entry from the cache
    \a key Configuration name
    @return
*/

/*!
@~english
    @fn QStringList DConfigCache::keyList() const = 0;
    @brief Returns all configuration items for the configuration content
    @return
*/

/*!
@~english
    @fn bool DConfigCache::setValue(const QString &key, const QVariant &value, const int serial, const uint uid, const QString &callerAppid) = 0;
    @brief Sets the value in the cache
    \a key Configuration name
    \a value Configuration name
    \a uid User Id at setup time
    \a callerAppid Application id at setup time
    @return A value of true indicates that the new value has been reset, and false indicates that it has not been set
*/

/*!
@~english
    @fn QVariant DConfigCache::value(const QString &key) const = 0;
    @brief Get the value in the cache
    \a key Configuration name
    @return
*/

/*!
@~english
    @fn int DConfigCache::serial(const QString &key) const = 0;
    @brief Returns a monotonically increasing value of a configuration item
    \a key Configuration name
    @return An invalid value of -1 indicates that the entry is not configured
*/

/*!
@~english
    @fn uint DConfigCache::uid() const = 0;
    @brief User identification, when used in the global cache, uid is a specific value for non-user identification
    @return
*/

/*!
@~english
    @fn void setCachePathPrefix(const QString &prefix) = 0;
    @brief Set cache's prefix path, it's access permissions is considered by caller,
and it needs to distinguish the paths of different caches by caller.
    @param prefix cache's prefix path.
*/

class Q_DECL_HIDDEN DConfigCacheImpl : public DConfigCache {
public:
    DConfigCacheImpl(const DConfigKey &configKey, const uint uid, bool global);
    virtual ~DConfigCacheImpl() override;

    // DConfigCache interface
public:
    inline virtual int serial(const QString &key) const override
    {
        return values.serial(key);
    }

    inline virtual uint uid() const override
    {
        return userid;
    }

    inline virtual QStringList keyList() const override
    {
        return values.keyList();
    }

    inline QString applicationCacheDir(const QString &localPrefix, const QString &suffix) const
    {
        QString prefix(cachePrefix);
        if (prefix.isEmpty()) {
            // If target user is current user or system user, then get the home path by environment variable first.
            QString homePath;
            if (userid == InvalidUID || (getuid() == userid)) {
                homePath = DStandardPaths::homePath();
            } else {
                homePath = DStandardPaths::homePath(getuid());
            }

            if (homePath.isEmpty())
                return QString();

            // fallback to default application cache directory.
            prefix = homePath + QStringLiteral("/.config/dsg/configs");
        }
        return QDir::cleanPath(QString("%1/%2/%3").arg(localPrefix, prefix + suffix, configKey.appId));
    }

    inline QString applicationCacheDir(const QString &localPrefix) const
    {
        return applicationCacheDir(localPrefix, QString());
    }

    inline QString cacheDir(const QString &basePath) {
        QDir dir(basePath + configKey.subpath);
        return dir.filePath(configKey.fileName + FILE_SUFFIX);
    }

    inline QString globalCacheDir(const QString &localPrefix) const {
        QString prefix(cachePrefix);
        if (prefix.isEmpty()) {
            // TODO `DSG_APP_DATA` is not set and `appid` is not captured in `DStandardPaths::path`.
            QString appDataDir = DStandardPaths::path(DStandardPaths::DSG::AppData);
            if (appDataDir.isEmpty()) {
#ifdef D_DSG_APP_DATA_FALLBACK
                appDataDir = QStringLiteral(D_DSG_APP_DATA_FALLBACK);
                QFileInfo tmp(appDataDir);
                if (!tmp.exists() && !tmp.isSymLink() && !QDir::current().mkpath(appDataDir)) {
                    qCDebug(cfLog, "Not found a valid DSG_APP_DATA directory");
                    return QString();
                }
#else
                return QString();
#endif
            }
            // fallback to default global cache directory.
            prefix = QString("%1/configs").arg(appDataDir);
        }

        return QDir::cleanPath(QString("%1/%2/%3").arg(localPrefix, prefix, configKey.appId));
    }

    QString getCacheDir(const QString &localPrefix = QString())
    {
        if (isGlobal()) {
            const QString &dir = globalCacheDir(localPrefix);
            if (!dir.isEmpty())
                return dir;

            // Not supported the global config, fallback the config cache data to user directory.
            return applicationCacheDir(localPrefix, "-fake-global");
        } else {
            return applicationCacheDir(localPrefix);
        }
    }

    bool load(const QString &localPrefix = QString()) override;

    bool isGlobal() const override
    {
        return global;
    }

    inline void remove(const QString &key) override
    {
        values.remove(key);
        cacheChanged = true;
    }
    bool setValue(const QString &key, const QVariant &value, const int serial, const uint uid, const QString &appid) override
    {
        if (values.value(key) == value) {
            return false;
        }
        values.setValue(key, value);
        values.setSerial(key, serial);
        values.setTime(key, QDateTime::currentDateTime().toString(Qt::ISODate));
        values.setUser(key, uid);
        values.setAppId(key, appid.isEmpty() ? configKey.appId : appid);
        cacheChanged = true;
        return true;
    }

    inline QVariant value(const QString &key) const override
    {
        return values.value(key);
    }

    bool save(const QString &localPrefix, QJsonDocument::JsonFormat format, bool sync) override;

    virtual void setCachePathPrefix(const QString &prefix) override
    {
        cachePrefix = prefix;
    }

    DConfigKey configKey;
    DConfigInfo values;
    QString cachePrefix;
    uint userid;
    bool global;
    bool cacheChanged = false;
};

DConfigCacheImpl::DConfigCacheImpl(const DConfigKey &configKey, const uint uid, bool global)
    : DConfigCache(),
      configKey(configKey),
      userid(uid),
      global(global)
{
}

DConfigCacheImpl::~DConfigCacheImpl()
{
}

bool DConfigCacheImpl::load(const QString &localPrefix)
{
    // cache 文件要严格匹配 subpath
    const QString &dir = getCacheDir(localPrefix);
    if (dir.isEmpty()) {
        return true;
    }
    QScopedPointer<QFile> cache(loadFile(dir,
                                         configKey.subpath,
                                         configKey.fileName + FILE_SUFFIX,
                                         false));
    if (!cache) {
        return true;
    }

    const QJsonDocument &doc = loadJsonFile(cache.data());

    if (doc.isObject()) {
        const QJsonObject &root = doc.object();
        if (!checkMagic(root, MAGIC_CACHE))
            return false;
        if (!checkVersion(root, DConfigFile::supportedVersion()))
            return false;

        auto &&contents = root[QLatin1String("contents")].toObject();
        auto i = contents.constBegin();

        // 原样保存原始数据
        for (; i != contents.constEnd(); ++i) {
            values.update(i.key(), i.value().toObject().toVariantHash());
        }
    }
    return true;
}

bool DConfigCacheImpl::save(const QString &localPrefix, QJsonDocument::JsonFormat format, bool sync)
{
    if (!cacheChanged)
        return true;

    cacheChanged = false;
    const QString &dir = getCacheDir(localPrefix);
    if (dir.isEmpty()) {
        qCWarning(cfLog, "Falied on saveing, the config cache directory is empty for the user[%d], "
                         "the current user[%d].", userid, getuid());
        return false;
    }
    QString path = cacheDir(dir);

    QFile cache(path);
    if (!QFile::exists(QFileInfo(cache.fileName()).path())) {
        QDir().mkpath(QFileInfo(cache.fileName()).path());
    }

    if (!cache.open(QIODevice::WriteOnly)) {
        qCWarning(cfLog, "Falied on saveing data when open file: \"%s\", error message: \"%s\"",
                  qPrintable(cache.fileName()), qPrintable(cache.errorString()));
        return false;
    }

    qCDebug(cfLog, "Save cache file \"%s\".", qPrintable(cache.fileName()));

    QJsonObject root;

    root[QLatin1String("magic")] = MAGIC_CACHE;
    const DConfigFile::Version version = DConfigFile::supportedVersion();
    root[QLatin1String("version")] = QString("%1.%2").arg(version.major)
            .arg(version.minor);

    root[QLatin1String("contents")] = values.content();
    QJsonDocument doc;
    doc.setObject(root);
    const QByteArray &json = doc.toJson(format);

    bool status = cache.write(json) == json.size();
    if (status && sync) {
        cache.flush();
    }
    return status;
}

class Q_DECL_HIDDEN DConfigFilePrivate : public DObjectPrivate {
public:
    DConfigFilePrivate(DConfigFile *qq, const QString &appId,
                       const QString &name, const QString &subpath)
        : DObjectPrivate(qq),
          configKey(appId, name ,subpath),
          configMeta(new DConfigMetaImpl(configKey))
    {
    }
    DConfigFilePrivate(DConfigFile *qq, const DConfigKey &configKey)
        : DObjectPrivate(qq),
          configKey(configKey),
          configMeta(new DConfigMetaImpl(configKey))
    {
    }

    ~DConfigFilePrivate() override;

    bool load(const QString &localPrefix)
    {
        bool status = configMeta->load(localPrefix);
        if (status) {
            // for cache
            status &= globalCache->load(localPrefix);
        }
        return status;
    }
    bool setValue(const QString &key, const QVariant &value,
                  DConfigCache *userCache, const QString &appid)
    {
        // 此处不要检查权限，在获取 value 时会检查
        if (auto cache = getCache(key, userCache)) {
            if (!value.isValid()) {
                cache->remove(key);
                return true;
            } else {
                const auto &metaValue = configMeta->value(key);
                // sample judgement to reduce a copy of convert.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                if (metaValue.typeId() == value.typeId())
                    return cache->setValue(key, value, configMeta->serial(key), cache->uid(), appid);

                // convert copy to meta's type, it promises `setValue` don't change meta's type.
                auto copy = value;
                if (!copy.convert(metaValue.metaType())) {
                    qCWarning(cfLog) << "check type error, meta type is " << metaValue.metaType().name()
                                     << ", and now type is " << value.metaType().name();
                    return false;
                }
#else
                if (metaValue.type() == value.type())
                    return cache->setValue(key, value, configMeta->serial(key), cache->uid(), appid);

                auto copy = value;
                if (!copy.convert(metaValue.userType())) {
                    qCWarning(cfLog) << "check type error, meta type is " << metaValue.type()
                                     << ", and now type is " << value.type();
                    return false;
                }
#endif

                return cache->setValue(key, copy, configMeta->serial(key), cache->uid(), appid);
            }
        }
        return false;
    }
    DConfigCache* getCache(const QString &key, DConfigCache *userCache) const
    {
        if(configMeta->flags(key).testFlag(DConfigFile::Global)) {
            return globalCache;
        }
        return userCache;
    }
    QVariant cacheValue(DConfigCache *userCache, const QString &key) const
    {
        // 检查权限
        if (configMeta->permissions(key) != DConfigFile::ReadOnly) {
            if (auto cache = getCache(key, userCache)) {
                if (DConfigInfo::checkSerial(configMeta->serial(key), cache->serial(key))) {
                    const QVariant &tmp = cache->value(key);
                    if (tmp.isValid())
                        return tmp;
                }
            }
        }
        return QVariant();
    }
    QVariant value(const QString &key, DConfigCache *userCache) const
    {
        const QVariant &v = cacheValue(userCache, key);
        if (v.isValid())
            return v;
        return configMeta->value(key);
    }

    D_DECLARE_PUBLIC(DConfigFile)

private:
    DConfigCacheImpl* globalCache;
    DConfigKey configKey;
    DConfigMeta *configMeta;
};

DConfigFilePrivate::~DConfigFilePrivate()
{
    if (globalCache) {
        delete globalCache;
        globalCache = nullptr;
    }
    if (configMeta) {
        delete configMeta;
        configMeta = nullptr;
    }
}

/*!
@~english
    @brief Supported versions
    @return
 */
constexpr DConfigFile::Version DConfigFile::supportedVersion()
{
    return DConfigFile::Version{1, 0};
}

/*!
@~english
    @brief 构造配置文件管理对象
    \a appId Application unique identification
    \a name Config filename
    \a subpath Subdirectories
 */
DConfigFile::DConfigFile(const QString &appId, const QString &name, const QString &subpath)
    : DObject(*new DConfigFilePrivate(this, appId, name, subpath))
{
    D_D(DConfigFile);
    d->globalCache = new DConfigCacheImpl(d->configKey, InvalidUID, true);
}

DConfigFile::DConfigFile(const DConfigFile &other)
    : DObject(*new DConfigFilePrivate(this, other.d_func()->configKey))
{
    D_D(DConfigFile);
    auto cache = new DConfigCacheImpl(d->configKey, InvalidUID, true);
    cache->values = other.d_func()->globalCache->values;
    d->globalCache = cache;
}

/*!
@~english
    @brief Parse configuration files
    \a localPrefix Directory prefix
    @return
*/
bool DConfigFile::load(const QString &localPrefix)
{
    D_D(DConfigFile);
    return d->load(localPrefix);
}

/*!
@~english
    @brief Parse the profile stream
    \a meta Prototype stream
    \a overrides File stream to find for the override mechanism
    @return
*/
bool DConfigFile::load(QIODevice *meta, const QList<QIODevice *> &overrides)
{
    return this->meta()->load(meta, overrides);
}

/*!
@~english
    @brief Save the cached value to disk
    \a format Save format
    \a sync Whether to refresh immediately
    @return
*/
bool DConfigFile::save(const QString &localPrefix, QJsonDocument::JsonFormat format, bool sync) const
{
    D_DC(DConfigFile);

    bool ok = d->globalCache->save(localPrefix, format, sync);

    return ok;
}

/*!
@~english
 * @brief DConfigFile::value
 * @param key Configuration name
 * @param userCache Specific user cache, \a userCache is unused when the key is global
 * @return
 */
QVariant DConfigFile::value(const QString &key, DConfigCache *userCache) const
{
    D_DC(DConfigFile);
    return d->value(key, userCache);
}

/*!
 * \brief DConfigFile::cacheValue Get a specific user cache's value
 * \param userCache Specific user cache, it is unused if the \a key is global configuration item
 * \param key Configuration name
 * \return
 */
QVariant DConfigFile::cacheValue(DConfigCache *userCache, const QString &key) const
{
    D_DC(DConfigFile);
    return d->cacheValue(userCache, key);
}

/*!
@~english
    @brief Sets the value in the cache
    \a key Configuration name
    \a value The value to set
    \a userCache Specific user cache at setup time
    \a appid Application id at setup time
    @return A value of true indicates that the new value has been reset, and false indicates that it has not been set
 */
bool DConfigFile::setValue(const QString &key, const QVariant &value, const QString &callerAppid, DConfigCache *userCache)
{
    D_D(DConfigFile);
    return d->setValue(key, value, userCache, callerAppid);
}

DConfigCache *DConfigFile::createUserCache(const uint uid)
{
    D_D(DConfigFile);
    return new DConfigCacheImpl(d->configKey, uid, false);
}


/*!
@~english
    @brief Return to the global cache
    @return
 */
DConfigCache *DConfigFile::globalCache() const
{
    D_DC(DConfigFile);
    return d->globalCache;
}

/*!
@~english
    @brief Return the prototype object
    @return
 */
DConfigMeta *DConfigFile::meta()
{
    D_D(DConfigFile);
    return d->configMeta;
}

/*!
@~english
    @brief Checks whether the configuration file is valid
    @return
 */
bool DConfigFile::isValid() const
{
    D_DC(DConfigFile);
    return versionIsValid(d->configMeta->version());
}

DCORE_END_NAMESPACE
