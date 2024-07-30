/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2020 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPARTS_PARTLOADER_H
#define KPARTS_PARTLOADER_H

#include <KPluginFactory>
#include <KPluginMetaData>
#include <QList>
#include <QObject>
#include <kparts/kparts_export.h>

namespace KParts
{
Q_NAMESPACE_EXPORT(KPARTS_EXPORT)

/**
 * Enum for standardized capabilities of KParts
 *
 * @code
 * {
 *  "KPlugin": {...},
 *  "KParts": {
 *      "Capabilities": [
 *          "ReadOnly"
 *      ],
 *      "InitialPreference": 42
 *  }
 * }
 * @endcode
 *
 * @since 6.4
 */
enum class PartCapability {
    ReadOnly = 1,
    ReadWrite = 2,
    BrowserView = 4,
};
Q_ENUM_NS(PartCapability)
Q_DECLARE_FLAGS(PartCapabilities, PartCapability)
Q_FLAG_NS(PartCapabilities)
Q_DECLARE_OPERATORS_FOR_FLAGS(PartCapabilities)

/**
 * Helper methods for locating and loading parts.
 * This is based upon KPluginFactory, but it takes
 * care of querying by mimetype, sorting the available parts by builtin
 * preference and by user preference.
 * @since 5.69
 */
namespace PartLoader
{
namespace Private
{

enum ErrorType {
    CouldNotLoadPlugin,
    NoPartFoundForMimeType,
    NoPartInstantiatedForMimeType,
};

/**
 * @internal
 * @param errorString translated, user-visible error string
 * @param errorText untranslated error text
 * @param argument argument for the text
 */
KPARTS_EXPORT void getErrorStrings(QString *errorString, QString *errorText, const QString &argument, ErrorType type);

}

/**
 * Parses the associated capabilities from the KPart. This parses the deprecated "ServiceTypes" array of the "KPlugin" object
 *
 * @since 6.4
 */
KPARTS_EXPORT PartCapabilities partCapabilities(const KPluginMetaData &data);

/**
 * Locate all available KParts using KPluginMetaData::findPlugins for a mimetype.
 * @return a list of plugin metadata, sorted by preference.
 * This takes care both of the builtin preference (set by developers)
 * and of user preference (stored in mimeapps.list).
 *
 * To load a part from one of the KPluginMetaData instances returned here,
 * use \ref instantiatePart()
 *
 * @since 5.69
 */
KPARTS_EXPORT QList<KPluginMetaData> partsForMimeType(const QString &mimeType);

/**
 * Attempts to create a KPart from the given metadata.
 *
 * @code
 * if (auto result = KParts::PartLoader::instantiatePart<MyPart>(metaData, parentWidget, parent, args)) {
 *     // result.plugin is valid and can be accessed
 * } else {
 *     // result contains information about the error
 * }
 * @endcode
 * @param data KPluginMetaData from which the plugin should be loaded
 * @param parentWidget The parent widget
 * @param parent The parent object
 * @param args A list of arguments to be passed to the part
 * @return Result object which contains the plugin instance and potentially error information
 * @since 5.100
 */
template<typename T>
static KPluginFactory::Result<T>
instantiatePart(const KPluginMetaData &data, QWidget *parentWidget = nullptr, QObject *parent = nullptr, const QVariantList &args = {})
{
    KPluginFactory::Result<T> result;
    KPluginFactory::Result<KPluginFactory> factoryResult = KPluginFactory::loadFactory(data);
    if (!factoryResult.plugin) {
        result.errorString = factoryResult.errorString;
        result.errorReason = factoryResult.errorReason;
        return result;
    }
    T *instance = factoryResult.plugin->create<T>(parentWidget, parent, args);
    if (!instance) {
        const QString fileName = data.fileName();
        Private::getErrorStrings(&result.errorString, &result.errorText, fileName, Private::CouldNotLoadPlugin);
        result.errorReason = KPluginFactory::INVALID_KPLUGINFACTORY_INSTANTIATION;
    } else {
        result.plugin = instance;
    }
    return result;
}

/**
 * Use this method to create a KParts part. It will try to create an object which inherits
 * @p T.
 *
 * @code
 * if (auto result = KParts::PartLoader::instantiatePartForMimeType<KParts::ReadOnlyPart>(mimeType, parentWidget, parent, args)) {
 *     // result.plugin is valid and can be accessed
 * } else {
 *     // result contains information about the error
 * }
 * @endcode
 *
 * @tparam T The interface for which an object should be created. The object will inherit @p T.
 * @param mimeType The mimetype for which we need a KParts.
 * @param parentWidget The parent widget for the part's widget.
 * @param parent The parent of the part.
 * @return Result object which contains the plugin instance and potentially error information
 * @since 5.100
 */
template<class T>
static KPluginFactory::Result<T>
instantiatePartForMimeType(const QString &mimeType, QWidget *parentWidget = nullptr, QObject *parent = nullptr, const QVariantList &args = {})
{
    const QList<KPluginMetaData> plugins = KParts::PartLoader::partsForMimeType(mimeType);
    if (plugins.isEmpty()) {
        KPluginFactory::Result<T> errorResult;
        errorResult.errorReason = KPluginFactory::ResultErrorReason::INVALID_PLUGIN;
        Private::getErrorStrings(&errorResult.errorString, &errorResult.errorText, mimeType, Private::NoPartFoundForMimeType);

        return errorResult;
    }

    for (const KPluginMetaData &plugin : plugins) {
        if (const auto result = instantiatePart<T>(plugin, parentWidget, parent, args)) {
            return result;
        }
    }

    KPluginFactory::Result<T> errorResult;
    errorResult.errorReason = KPluginFactory::ResultErrorReason::INVALID_PLUGIN;
    Private::getErrorStrings(&errorResult.errorString, &errorResult.errorText, mimeType, Private::NoPartInstantiatedForMimeType);

    return errorResult;
}

} // namespace
} // namespace

Q_DECLARE_METATYPE(KParts::PartCapabilities)

#endif
