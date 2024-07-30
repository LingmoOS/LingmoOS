/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpluginmetadata.h"
#include "kstaticpluginhelpers_p.h"

#include "kcoreaddons_debug.h"
#include "kjsonutils.h"
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocale>
#include <QMimeDatabase>
#include <QPluginLoader>
#include <QStandardPaths>

#include "kaboutdata.h"

#include <optional>
#include <unordered_map>

using PluginCache = std::unordered_map<QString, std::vector<KPluginMetaData>>;
Q_GLOBAL_STATIC(PluginCache, s_pluginNamespaceCache)

class KPluginMetaDataPrivate : public QSharedData
{
public:
    KPluginMetaDataPrivate(const QJsonObject &obj, const QString &fileName, KPluginMetaData::KPluginMetaDataOptions options = {})
        : m_metaData(obj)
        , m_rootObj(obj.value(QLatin1String("KPlugin")).toObject())
        , m_fileName(fileName)
        , m_options(options)
    {
    }
    const QJsonObject m_metaData;
    const QJsonObject m_rootObj;
    // If we want to load a file, but it does not exist we want to keep the requested file name for logging
    QString m_requestedFileName;
    const QString m_fileName;
    const KPluginMetaData::KPluginMetaDataOptions m_options;
    std::optional<QStaticPlugin> staticPlugin = std::nullopt;
    // We determine this once and reuse the value. It can never change during the
    // lifetime of the KPluginMetaData object
    QString m_pluginId;
    qint64 m_lastQueriedTs = 0;

    static void forEachPlugin(const QString &directory, std::function<void(const QFileInfo &)> callback)
    {
        QStringList dirsToCheck;
#ifdef Q_OS_ANDROID
        dirsToCheck << QCoreApplication::libraryPaths();
#else
        if (QDir::isAbsolutePath(directory)) {
            dirsToCheck << directory;
        } else {
            dirsToCheck = QCoreApplication::libraryPaths();
            const QString appDirPath = QCoreApplication::applicationDirPath();
            dirsToCheck.removeOne(appDirPath);
            dirsToCheck.prepend(appDirPath);

            for (QString &libDir : dirsToCheck) {
                libDir += QLatin1Char('/') + directory;
            }
        }
#endif

        qCDebug(KCOREADDONS_DEBUG) << "Checking for plugins in" << dirsToCheck;

        for (const QString &dir : std::as_const(dirsToCheck)) {
            QDirIterator it(dir, QDir::Files);
            while (it.hasNext()) {
                it.next();
#ifdef Q_OS_ANDROID
                QString prefix(QLatin1String("libplugins_") + QString(directory).replace(QLatin1Char('/'), QLatin1String("_")));
                if (!prefix.endsWith(QLatin1Char('_'))) {
                    prefix.append(QLatin1Char('_'));
                }
                if (it.fileName().startsWith(prefix) && QLibrary::isLibrary(it.fileName())) {
#else
                if (QLibrary::isLibrary(it.fileName())) {
#endif
                    callback(it.fileInfo());
                }
            }
        }
    }

    struct StaticPluginLoadResult {
        QString fileName;
        QJsonObject metaData;
    };
    // This is only relevant in the findPlugins context and thus internal API.
    // If one has a static plugin from QPluginLoader::staticPlugins and does not
    // want it to have metadata, using KPluginMetaData makes no sense
    static KPluginMetaData
    ofStaticPlugin(const QString &pluginNamespace, const QString &fileName, KPluginMetaData::KPluginMetaDataOptions options, QStaticPlugin plugin)
    {
        QString pluginPath = pluginNamespace + u'/' + fileName;
        auto d = new KPluginMetaDataPrivate(plugin.metaData().value(QLatin1String("MetaData")).toObject(), pluginPath, options);
        d->staticPlugin = plugin;
        d->m_pluginId = fileName;
        KPluginMetaData data;
        data.d = d;
        return data;
    }
    static void pluginLoaderForPath(QPluginLoader &loader, const QString &path)
    {
        if (path.startsWith(QLatin1Char('/'))) { // Absolute path, use as it is
            loader.setFileName(path);
        } else {
            loader.setFileName(QCoreApplication::applicationDirPath() + QLatin1Char('/') + path);
            if (loader.fileName().isEmpty()) {
                loader.setFileName(path);
            }
        }
    }

    static KPluginMetaDataPrivate *ofPath(const QString &path, KPluginMetaData::KPluginMetaDataOptions options)
    {
        QPluginLoader loader;
        pluginLoaderForPath(loader, path);
        if (loader.metaData().isEmpty()) {
            qCDebug(KCOREADDONS_DEBUG) << "no metadata found in" << loader.fileName() << loader.errorString();
        }
        auto ret = new KPluginMetaDataPrivate(loader.metaData().value(QLatin1String("MetaData")).toObject(), //
                                              QFileInfo(loader.fileName()).absoluteFilePath(),
                                              options);
        ret->m_requestedFileName = path;
        return ret;
    }
};

KPluginMetaData::KPluginMetaData()
    : d(new KPluginMetaDataPrivate(QJsonObject(), QString()))
{
}

KPluginMetaData::KPluginMetaData(const KPluginMetaData &other)
    : d(other.d)
{
}

KPluginMetaData &KPluginMetaData::operator=(const KPluginMetaData &other)
{
    d = other.d;
    return *this;
}

KPluginMetaData::~KPluginMetaData() = default;

KPluginMetaData::KPluginMetaData(const QString &pluginFile, KPluginMetaDataOptions options)
    : d(KPluginMetaDataPrivate::ofPath(pluginFile, options))
{
    // passing QFileInfo an empty string gives the CWD, which is not what we want
    if (!d->m_fileName.isEmpty()) {
        d->m_pluginId = QFileInfo(d->m_fileName).completeBaseName();
    }

    if (d->m_metaData.isEmpty() && !options.testFlags(KPluginMetaDataOption::AllowEmptyMetaData)) {
        qCDebug(KCOREADDONS_DEBUG) << "plugin metadata in" << pluginFile << "does not have a valid 'MetaData' object";
    }
    if (const QString id = d->m_rootObj[QLatin1String("Id")].toString(); !id.isEmpty()) {
        if (id != d->m_pluginId) {
            qWarning(KCOREADDONS_DEBUG) << "The plugin" << pluginFile
                                        << "explicitly states an Id in the embedded metadata, which is different from the one derived from the filename"
                                        << "The Id field from the KPlugin object in the metadata should be removed";
        } else {
            qInfo(KCOREADDONS_DEBUG) << "The plugin" << pluginFile << "explicitly states an 'Id' in the embedded metadata."
                                     << "This value should be removed, the resulting pluginId will not be affected by it";
        }
    }
}

KPluginMetaData::KPluginMetaData(const QPluginLoader &loader, KPluginMetaDataOptions options)
    : d(new KPluginMetaDataPrivate(loader.metaData().value(QLatin1String("MetaData")).toObject(), loader.fileName(), options))
{
    if (!loader.fileName().isEmpty()) {
        d->m_pluginId = QFileInfo(loader.fileName()).completeBaseName();
    }
}

KPluginMetaData::KPluginMetaData(const QJsonObject &metaData, const QString &fileName)
    : d(new KPluginMetaDataPrivate(metaData, fileName))
{
    auto nameFromMetaData = d->m_rootObj.constFind(QStringLiteral("Id"));
    if (nameFromMetaData != d->m_rootObj.constEnd()) {
        d->m_pluginId = nameFromMetaData.value().toString();
    }
    if (d->m_pluginId.isEmpty()) {
        d->m_pluginId = QFileInfo(d->m_fileName).completeBaseName();
    }
}

KPluginMetaData KPluginMetaData::findPluginById(const QString &directory, const QString &pluginId, KPluginMetaDataOptions options)
{
    QPluginLoader loader;
    const QString fileName = directory + QLatin1Char('/') + pluginId;
    KPluginMetaDataPrivate::pluginLoaderForPath(loader, fileName);
    if (loader.load()) {
        if (KPluginMetaData metaData(loader, options); metaData.isValid()) {
            return metaData;
        }
    }

    if (const auto staticOptional = KStaticPluginHelpers::findById(directory, pluginId)) {
        KPluginMetaData data = KPluginMetaDataPrivate::ofStaticPlugin(directory, pluginId, options, staticOptional.value());
        Q_ASSERT(data.fileName() == fileName);
        return data;
    }

    return KPluginMetaData{};
}

KPluginMetaData KPluginMetaData::fromJsonFile(const QString &file)
{
    QFile f(file);
    bool b = f.open(QIODevice::ReadOnly);
    if (!b) {
        qCWarning(KCOREADDONS_DEBUG) << "Couldn't open" << file;
        return {};
    }
    QJsonParseError error;
    const QJsonObject metaData = QJsonDocument::fromJson(f.readAll(), &error).object();
    if (error.error) {
        qCWarning(KCOREADDONS_DEBUG) << "error parsing" << file << error.errorString();
    }

    return KPluginMetaData(metaData, QFileInfo(file).absoluteFilePath());
}

QJsonObject KPluginMetaData::rawData() const
{
    return d->m_metaData;
}

QString KPluginMetaData::fileName() const
{
    return d->m_fileName;
}
QList<KPluginMetaData>
KPluginMetaData::findPlugins(const QString &directory, std::function<bool(const KPluginMetaData &)> filter, KPluginMetaDataOptions options)
{
    QList<KPluginMetaData> ret;
    const auto staticPlugins = KStaticPluginHelpers::staticPlugins(directory);
    for (auto it = staticPlugins.begin(); it != staticPlugins.end(); ++it) {
        KPluginMetaData metaData = KPluginMetaDataPrivate::ofStaticPlugin(directory, it.key(), options, it.value());
        if (metaData.isValid()) {
            if (!filter || filter(metaData)) {
                ret << metaData;
            }
        }
    }
    QSet<QString> addedPluginIds;
    const qint64 nowTs = QDateTime::currentMSecsSinceEpoch(); // For the initial load, stating all files is not needed
    const bool checkCache = options.testFlags(KPluginMetaData::CacheMetaData);
    std::vector<KPluginMetaData> &cache = (*s_pluginNamespaceCache)[directory];
    KPluginMetaDataPrivate::forEachPlugin(directory, [&](const QFileInfo &pluginInfo) {
        const QString pluginFile = pluginInfo.absoluteFilePath();

        KPluginMetaData metadata;
        if (checkCache) {
            const auto it = std::find_if(cache.begin(), cache.end(), [&pluginFile](const KPluginMetaData &data) {
                return pluginFile == data.fileName();
            });
            bool isNew = it == cache.cend();
            if (!isNew) {
                const qint64 lastQueried = (*it).d->m_lastQueriedTs;
                Q_ASSERT(lastQueried > 0);
                isNew = lastQueried < pluginInfo.lastModified().toMSecsSinceEpoch();
            }
            if (!isNew) {
                metadata = *it;
            } else {
                metadata = KPluginMetaData(pluginFile, options);
                metadata.d->m_lastQueriedTs = nowTs;
                cache.push_back(metadata);
            }
        } else {
            metadata = KPluginMetaData(pluginFile, options);
        }
        if (!metadata.isValid()) {
            qCDebug(KCOREADDONS_DEBUG) << pluginFile << "does not contain valid JSON metadata";
            return;
        }
        if (addedPluginIds.contains(metadata.pluginId())) {
            return;
        }
        if (filter && !filter(metadata)) {
            return;
        }
        addedPluginIds << metadata.pluginId();
        ret.append(metadata);
    });
    return ret;
}

bool KPluginMetaData::isValid() const
{
    // it can be valid even if m_fileName is empty (as long as the plugin id is
    // set)
    return !pluginId().isEmpty() && (!d->m_metaData.isEmpty() || d->m_options.testFlags(AllowEmptyMetaData));
}

bool KPluginMetaData::isHidden() const
{
    return d->m_rootObj[QLatin1String("Hidden")].toBool();
}

static inline void addPersonFromJson(const QJsonObject &obj, QList<KAboutPerson> *out)
{
    KAboutPerson person = KAboutPerson::fromJSON(obj);
    if (person.name().isEmpty()) {
        qCWarning(KCOREADDONS_DEBUG) << "Invalid plugin metadata: Attempting to create a KAboutPerson from JSON without 'Name' property:" << obj;
        return;
    }
    out->append(person);
}

static QList<KAboutPerson> aboutPersonFromJSON(const QJsonValue &people)
{
    QList<KAboutPerson> ret;
    if (people.isObject()) {
        // single author
        addPersonFromJson(people.toObject(), &ret);
    } else if (people.isArray()) {
        const QJsonArray peopleArray = people.toArray();
        for (const QJsonValue &val : peopleArray) {
            if (val.isObject()) {
                addPersonFromJson(val.toObject(), &ret);
            }
        }
    }
    return ret;
}

QList<KAboutPerson> KPluginMetaData::authors() const
{
    return aboutPersonFromJSON(d->m_rootObj[QLatin1String("Authors")]);
}

QList<KAboutPerson> KPluginMetaData::translators() const
{
    return aboutPersonFromJSON(d->m_rootObj[QLatin1String("Translators")]);
}

QList<KAboutPerson> KPluginMetaData::otherContributors() const
{
    return aboutPersonFromJSON(d->m_rootObj[QLatin1String("OtherContributors")]);
}

QString KPluginMetaData::category() const
{
    return d->m_rootObj[QLatin1String("Category")].toString();
}

QString KPluginMetaData::description() const
{
    return KJsonUtils::readTranslatedString(d->m_rootObj, QStringLiteral("Description"));
}

QString KPluginMetaData::iconName() const
{
    return d->m_rootObj[QLatin1String("Icon")].toString();
}

QString KPluginMetaData::license() const
{
    return d->m_rootObj[QLatin1String("License")].toString();
}

QString KPluginMetaData::licenseText() const
{
    return KAboutLicense::byKeyword(license()).text();
}

QString KPluginMetaData::name() const
{
    return KJsonUtils::readTranslatedString(d->m_rootObj, QStringLiteral("Name"));
}

QString KPluginMetaData::copyrightText() const
{
    return KJsonUtils::readTranslatedString(d->m_rootObj, QStringLiteral("Copyright"));
}

QString KPluginMetaData::pluginId() const
{
    return d->m_pluginId;
}

QString KPluginMetaData::version() const
{
    return d->m_rootObj[QLatin1String("Version")].toString();
}

QString KPluginMetaData::website() const
{
    return d->m_rootObj[QLatin1String("Website")].toString();
}

QString KPluginMetaData::bugReportUrl() const
{
    return d->m_rootObj[QLatin1String("BugReportUrl")].toString();
}

QStringList KPluginMetaData::mimeTypes() const
{
    return d->m_rootObj[QLatin1String("MimeTypes")].toVariant().toStringList();
}

bool KPluginMetaData::supportsMimeType(const QString &mimeType) const
{
    // Check for exact matches first. This can delay parsing the full MIME
    // database until later and noticeably speed up application startup on
    // slower systems.
    const QStringList mimes = mimeTypes();
    if (mimes.contains(mimeType)) {
        return true;
    }

    // Now check for MIME type inheritance to find non-exact matches:
    QMimeDatabase db;
    const QMimeType mime = db.mimeTypeForName(mimeType);
    if (!mime.isValid()) {
        return false;
    }

    return std::any_of(mimes.begin(), mimes.end(), [&](const QString &supportedMimeName) {
        return mime.inherits(supportedMimeName);
    });
}

QStringList KPluginMetaData::formFactors() const
{
    return d->m_rootObj.value(QLatin1String("FormFactors")).toVariant().toStringList();
}

bool KPluginMetaData::isEnabledByDefault() const
{
    const QLatin1String key("EnabledByDefault");
    const QJsonValue val = d->m_rootObj[key];
    if (val.isBool()) {
        return val.toBool();
    } else if (val.isString()) {
        qCWarning(KCOREADDONS_DEBUG) << "Expected JSON property" << key << "in" << d->m_fileName << "to be boolean, but it was a string";
        return val.toString() == QLatin1String("true");
    }
    return false;
}

QString KPluginMetaData::value(const QString &key, const QString &defaultValue) const
{
    const QJsonValue value = d->m_metaData.value(key);
    if (value.isString()) {
        return value.toString(defaultValue);
    } else if (value.isArray()) {
        qCWarning(KCOREADDONS_DEBUG) << "Expected JSON property" << key << "in" << d->m_fileName << "to be a single string, but it is an array";
        return value.toVariant().toStringList().join(QChar::fromLatin1(','));
    } else if (value.isBool()) {
        qCWarning(KCOREADDONS_DEBUG) << "Expected JSON property" << key << "in" << d->m_fileName << "to be a single string, but it is a bool";
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    }
    return defaultValue;
}

bool KPluginMetaData::value(const QString &key, bool defaultValue) const
{
    const QJsonValue value = d->m_metaData.value(key);
    if (value.isBool()) {
        return value.toBool();
    } else if (value.isString()) {
        return value.toString() == QLatin1String("true");
    } else {
        return defaultValue;
    }
}

int KPluginMetaData::value(const QString &key, int defaultValue) const
{
    const QJsonValue value = d->m_metaData.value(key);
    if (value.isDouble()) {
        return value.toInt();
    } else if (value.isString()) {
        const QString intString = value.toString();
        bool ok;
        int convertedIntValue = intString.toInt(&ok);
        if (ok) {
            return convertedIntValue;
        } else {
            qCWarning(KCOREADDONS_DEBUG) << "Expected" << key << "to be an int, instead" << intString << "was specified in the JSON metadata" << d->m_fileName;
            return defaultValue;
        }
    } else {
        return defaultValue;
    }
}
QStringList KPluginMetaData::value(const QString &key, const QStringList &defaultValue) const
{
    const QJsonValue value = d->m_metaData.value(key);
    if (value.isUndefined() || value.isNull()) {
        return defaultValue;
    } else if (value.isObject()) {
        qCWarning(KCOREADDONS_DEBUG) << "Expected JSON property" << key << "to be a string list, instead an object was specified in" << d->m_fileName;
        return defaultValue;
    } else if (value.isArray()) {
        return value.toVariant().toStringList();
    } else {
        const QString asString = value.isString() ? value.toString() : value.toVariant().toString();
        if (asString.isEmpty()) {
            return defaultValue;
        }
        qCDebug(KCOREADDONS_DEBUG) << "Expected JSON property" << key << "to be a string list in" << d->m_fileName
                                   << "Treating it as a list with a single entry:" << asString;
        return QStringList(asString);
    }
}

bool KPluginMetaData::operator==(const KPluginMetaData &other) const
{
    return d->m_fileName == other.d->m_fileName && d->m_metaData == other.d->m_metaData;
}

bool KPluginMetaData::isStaticPlugin() const
{
    return d->staticPlugin.has_value();
}

QString KPluginMetaData::requestedFileName() const
{
    return d->m_requestedFileName;
}

QStaticPlugin KPluginMetaData::staticPlugin() const
{
    Q_ASSERT(d);
    Q_ASSERT(d->staticPlugin.has_value());
    return d->staticPlugin.value();
}

QDebug operator<<(QDebug debug, const KPluginMetaData &metaData)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "KPluginMetaData(pluginId:" << metaData.pluginId() << ", fileName: " << metaData.fileName() << ')';
    return debug;
}

#include "moc_kpluginmetadata.cpp"
