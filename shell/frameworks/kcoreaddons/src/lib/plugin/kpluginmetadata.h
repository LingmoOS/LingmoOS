/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINMETADATA_H
#define KPLUGINMETADATA_H

#include "kcoreaddons_export.h"

#include <QExplicitlySharedDataPointer>
#include <QJsonObject>
#include <QMetaType>
#include <QString>
#include <QStringList>

#include <functional>

class QPluginLoader;
class QStaticPlugin;
class KPluginMetaDataPrivate;
class KAboutPerson;
/**
 * @class KPluginMetaData kpluginmetadata.h KPluginMetaData
 *
 * This class allows easily accessing some standardized values from the JSON metadata that
 * can be embedded into Qt plugins. Additional plugin-specific metadata can be retrieved by
 * directly reading from the QJsonObject returned by KPluginMetaData::rawData().
 *
 * For embedded metadata, you should not specify an id manually. Instead the id will
 * be derived from the file basename.
 *
 * The following keys will be read from an object "KPlugin" inside the metadata JSON:
 *
 * Key                | Accessor function    | JSON Type
 * -------------------| -------------------- | ---------------------
 * Name               | name()               | string
 * Description        | description()        | string
 * Icon               | iconName()           | string
 * Authors            | authors()            | object array (KAboutPerson)
 * Category           | category()           | string
 * License            | license()            | string
 * Copyright          | copyrightText()      | string
 * Id                 | pluginId()           | string
 * Version            | version()            | string
 * Website            | website()            | string
 * BugReportUrl       | bugReportUrl()       | string
 * EnabledByDefault   | isEnabledByDefault() | bool
 * MimeTypes          | mimeTypes()          | string array
 * FormFactors        | formFactors()        | string array
 * Translators        | translators()        | object array (KAboutPerson)
 * OtherContributors  | otherContributors()  | object array (KAboutPerson)
 *
 * The Authors, Translators and OtherContributors keys are expected to be
 * list of objects that match the structure expected by KAboutPerson::fromJSON().
 *
 * An example metadata json file could look like this:
 * @verbatim
   {
     "KPlugin": {
        "Name": "Date and Time",
        "Description": "Date and time by timezone",
        "Icon": "preferences-system-time",
        "Authors": [ { "Name": "Aaron Seigo", "Email": "aseigo@kde.org" } ],
        "Category": "Date and Time",
        "EnabledByDefault": "true",
        "License": "LGPL",
        "Version": "1.0",
        "Website": "https://lingmo.kde.org/"
     }
   }
   @endverbatim
 *
 * @sa KAboutPerson::fromJSON()
 * @since 5.1
 */
class KCOREADDONS_EXPORT KPluginMetaData
{
    Q_GADGET
    Q_PROPERTY(bool isValid READ isValid CONSTANT)
    Q_PROPERTY(bool isHidden READ isHidden CONSTANT)
    Q_PROPERTY(QString fileName READ fileName CONSTANT)
    Q_PROPERTY(QJsonObject rawData READ rawData CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QList<KAboutPerson> authors READ authors CONSTANT)
    Q_PROPERTY(QList<KAboutPerson> translators READ translators CONSTANT)
    Q_PROPERTY(QList<KAboutPerson> otherContributors READ otherContributors CONSTANT)
    Q_PROPERTY(QString category READ category CONSTANT)
    Q_PROPERTY(QString iconName READ iconName CONSTANT)
    Q_PROPERTY(QString license READ license CONSTANT)
    Q_PROPERTY(QString licenseText READ licenseText CONSTANT)
    Q_PROPERTY(QString copyrightText READ copyrightText CONSTANT)
    Q_PROPERTY(QString pluginId READ pluginId CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString website READ website CONSTANT)
    Q_PROPERTY(QString bugReportUrl READ bugReportUrl CONSTANT)
    Q_PROPERTY(QStringList mimeTypes READ mimeTypes CONSTANT)
    Q_PROPERTY(QStringList formFactors READ formFactors CONSTANT)
    Q_PROPERTY(bool isEnabledByDefault READ isEnabledByDefault CONSTANT)

public:
    /**
     * Options for creating a KPluginMetaData object.
     * @since 5.91
     */
    enum KPluginMetaDataOption {
        AllowEmptyMetaData = 1, ///< Plugins with empty metaData are considered valid
        /**
         * If KCoreAddons should keep metadata in cache. This makes querying the namespace again faster. Consider using this if you need revalidation of plugins
         * @since 6.0
         */
        CacheMetaData = 2,
    };
    Q_DECLARE_FLAGS(KPluginMetaDataOptions, KPluginMetaDataOption)
    Q_FLAG(KPluginMetaDataOption)

    /** Creates an invalid KPluginMetaData instance */
    KPluginMetaData();

    /**
     * Reads the plugin metadata from a QPluginLoader instance. You must call QPluginLoader::setFileName()
     * or use the appropriate constructor on @p loader before calling this.
     * @param option Added in 6.0, see enum docs
     */
    KPluginMetaData(const QPluginLoader &loader, KPluginMetaDataOptions options = {});

    /**
     * Reads the plugin metadata from a plugin which can be loaded from @p file.
     *
     * Platform-specific library suffixes may be omitted since @p file will be resolved
     * using the same logic as QPluginLoader.
     *
     * @see QPluginLoader::setFileName()
     */
    KPluginMetaData(const QString &pluginFile, KPluginMetaDataOptions options = {});

    /**
     * Creates a KPluginMetaData from a QJsonObject holding the metadata and a file name
     * This can be used if the data is not retrieved from a Qt C++ plugin library but from some
     * other source.
     *
     * @param metaData the JSON metadata to use for this object
     * @param pluginFile the file that the plugin can be loaded from
     *
     * @since 6.0
     */
    KPluginMetaData(const QJsonObject &metaData, const QString &fileName);

    /**
     * Copy contructor
     */
    KPluginMetaData(const KPluginMetaData &);
    /**
     * Copy assignment
     */
    KPluginMetaData &operator=(const KPluginMetaData &);
    /**
     * Destructor
     */
    ~KPluginMetaData();

    /**
     * Load a KPluginMetaData instance from a .json file. Unlike the constructor with a single file argument,
     * this ensure that only JSON format plugins are loaded and any other type is rejected.
     *
     * @param jsonFile the .json file to load
     * @since 5.91
     */
    static KPluginMetaData fromJsonFile(const QString &jsonFile);

    /**
     * @param directory The directory to search for plugins. If a relative path is given for @p directory,
     * all entries of QCoreApplication::libraryPaths() will be checked with @p directory appended as a
     * subdirectory. If an absolute path is given only that directory will be searched.
     * @note Check if the returned KPluginMetaData is valid before continuing to use it.
     *
     * @param pluginId The Id of the plugin. The id should be the same as the filename, see KPluginMetaData::pluginId()
     * @param option Added in 6.0, see enum docs
     * @since 5.84
     */
    static KPluginMetaData findPluginById(const QString &directory, const QString &pluginId, KPluginMetaDataOptions options = {});

    /**
     * Find all plugins inside @p directory. Only plugins which have JSON metadata will be considered.
     *
     * @param directory The directory to search for plugins. If a relative path is given for @p directory,
     * all entries of QCoreApplication::libraryPaths() will be checked with @p directory appended as a
     * subdirectory. If an absolute path is given only that directory will be searched.
     *
     * @param filter a callback function that returns @c true if the found plugin should be loaded
     * and @c false if it should be skipped. If this argument is omitted all plugins will be loaded
     * @param option Weather or not allow plugins with empty metadata to be considered valid
     *
     * @return all plugins found in @p directory that fulfil the constraints of @p filter
     * @since 5.86
     */
    static QList<KPluginMetaData>
    findPlugins(const QString &directory, std::function<bool(const KPluginMetaData &)> filter = {}, KPluginMetaDataOptions options = {});

    /**
     * @return whether this object holds valid information about a plugin.
     * If this is @c true pluginId() will return a non-empty string.
     */
    bool isValid() const;

    /**
     * @return whether this object should be hidden
     *
     * @since 5.8
     */
    bool isHidden() const;

    /**
     * @return the path to the plugin.
     * When the KPluginMetaData(QJsonObject, QString) constructor is used, the string is not modified.
     * Otherwise, the path is resolved using QPluginLoader.
     * For static plugins the fileName is the namespace and pluginId concatenated
     *
     * @note It is not guaranteed that this is a valid path to a shared library (i.e. loadable
     * by QPluginLoader) since the metadata could also refer to a non-C++ plugin.
     */
    QString fileName() const;

    /**
     * @return the full metadata stored inside the plugin file.
     */
    QJsonObject rawData() const;

    /**
     * @return the user visible name of the plugin.
     */
    QString name() const;

    /**
     * @return a short description of the plugin.
     */
    QString description() const;

    /**
     * @return the author(s) of this plugin.
     */
    QList<KAboutPerson> authors() const;

    /**
     * @return the translator(s) of this plugin.
     *
     * @since 5.18
     */
    QList<KAboutPerson> translators() const;

    /**
     * @return a list of people that contributed to this plugin (other than the authors and translators).
     *
     * @since 5.18
     */
    QList<KAboutPerson> otherContributors() const;

    /**
     * @return the categories of this plugin (e.g. "playlist/skin").
     */
    QString category() const;

    /**
     * @return the icon name for this plugin
     * @see QIcon::fromTheme()
     */
    QString iconName() const;

    /**
     * @return the short license identifier (e.g. LGPL).
     * @see KAboutLicense::byKeyword() for retrieving the full license information
     */
    QString license() const;

    /**
     * @return the text of the license, equivalent to KAboutLicense::byKeyword(license()).text()
     * @since 5.73
     */
    QString licenseText() const;

    /**
     * @return a short copyright statement
     *
     * @since 5.18
     */
    QString copyrightText() const;

    /**
     * @return the unique identifier within the namespace of the plugin
     *
     * For C++ plugins, this ID is derived from the filename.
     * It should not be set in the metadata explicitly.
     *
     * When using @ref KPluginMetaData::fromJsonFile or @ref KPluginMetaData(QJsonObject, QString),
     * the "Id" of the "KPlugin" object will be used. If unset, it will be derived
     * from the filename.
     */
    QString pluginId() const;

    /**
     * @return the version of the plugin.
     */
    QString version() const;

    /**
     * @return the website of the plugin.
     */
    QString website() const;

    /**
     * @return the website where people can report a bug found in this plugin
     * @since 5.99
     */
    QString bugReportUrl() const;

    /**
     * @return a list of MIME types this plugin can handle (e.g. "application/pdf", "image/png", etc.)
     * @since 5.16
     */
    QStringList mimeTypes() const;

    /**
     * @return true if this plugin can handle the given mimetype
     * This is more accurate than mimeTypes().contains(mimeType) because it also
     * takes MIME type inheritance into account.
     * @since 5.66
     */
    bool supportsMimeType(const QString &mimeType) const;

    /**
     * @return A string list of formfactors this plugin is useful for, e.g. desktop, handset or mediacenter.
     * The keys for this are not formally defined, though the above-mentioned values should be used when applicable.
     *
     * @since 5.12
     */
    QStringList formFactors() const;

    /**
     * @return whether the plugin should be enabled by default.
     * This is only a recommendation, applications can ignore this value if they want to.
     */
    bool isEnabledByDefault() const;

    /**
     * Returns @c true if the plugin is enabled in @p config, otherwise returns isEnabledByDefault().
     * This can be used in conjunction with KPluginWidget.
     *
     * The @p config param should be a KConfigGroup object, because KCoreAddons can not depend
     * on KConfig directly, this parameter is a template.
     * @param config KConfigGroup where the enabled state is stored
     * @since 5.89
     */
    template<typename T>
    bool isEnabled(const T &config) const
    {
        Q_ASSERT(config.isValid());
        return config.readEntry(pluginId() + QLatin1String("Enabled"), isEnabledByDefault());
    }

    /**
     * @return the string value for @p key from the metadata or @p defaultValue if the key does not exist
     *
     * if QString is not the correct type for @p key you should use the other overloads or @ref KPluginMetaData::rawData
     */
    QString value(const QString &key, const QString &defaultValue = QString()) const;
    QString value(const QString &key, const char *ch) const = delete;

    /**
     * @overload
     * @since 5.88
     */
    bool value(const QString &key, bool defaultValue) const;

    /**
     * @overload
     * @since 5.88
     */
    int value(const QString &key, int defaultValue) const;

    /** @return the value for @p key from the metadata or @p defaultValue if the key does not exist.
     * If the type of @p key is string, a list containing just that string will be returned.
     * If the type is array, the list will contain one entry for each array member.
     * @overload
     * @since 5.88
     */
    QStringList value(const QString &key, const QStringList &defaultValue) const;

    /**
     * @return @c true if this object is equal to @p other, otherwise @c false
     */
    bool operator==(const KPluginMetaData &other) const;

    /**
     * @return @c true if this object is not equal to @p other, otherwise @c false.
     */
    inline bool operator!=(const KPluginMetaData &other) const
    {
        return !(*this == other);
    }

    /**
     * @note for loading plugin the plugin independently of it being static/dynamic
     * use @ref KPluginFactory::loadFactory or @ref KPluginFactory::instantiatePlugin.
     * @return true if the instance represents a static plugin
     * @since 5.89
     */
    bool isStaticPlugin() const;

private:
    KCOREADDONS_NO_EXPORT QStaticPlugin staticPlugin() const;
    KCOREADDONS_NO_EXPORT QString requestedFileName() const;

    QExplicitlySharedDataPointer<KPluginMetaDataPrivate> d;
    friend class KPluginFactory;
    friend class KPluginMetaDataPrivate;
};

inline size_t qHash(const KPluginMetaData &md, size_t seed)
{
    return qHash(md.pluginId(), seed);
}

/// @since 6.0
KCOREADDONS_EXPORT QDebug operator<<(QDebug debug, const KPluginMetaData &metaData);

Q_DECLARE_TYPEINFO(KPluginMetaData, Q_RELOCATABLE_TYPE);
Q_DECLARE_OPERATORS_FOR_FLAGS(KPluginMetaData::KPluginMetaDataOptions)

#endif // KPLUGINMETADATA_H
