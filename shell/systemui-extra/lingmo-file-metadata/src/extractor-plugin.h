/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef _LINGMOFILEMETADATA_EXTRACTOR_PLUGIN_H
#define _LINGMOFILEMETADATA_EXTRACTOR_PLUGIN_H

#include <QStringList>
#include <QDateTime>

#include "lingmo-file-metadata_global.h"
#include "extraction-result.h"

namespace LingmoUIFileMetadata
{

/**
 * \class ExtractorPlugin extractorplugin.h
 *
 * \brief The ExtractorPlugin is the base class for all file metadata
 * extractors. It is responsible for extracting the metadata in a file.
 *
 * Plugins should derive from this class and implement the mimetypes
 * and extract method.
 *
 * All Plugins should be synchronous and blocking.
 *
 */
class LINGMOFILEMETADATA_EXPORT ExtractorPlugin : public QObject
{
    Q_OBJECT
public:
    explicit ExtractorPlugin(QObject* parent);
    ~ExtractorPlugin() override;

    /**
     * Provide a list of mimetypes which are supported by this plugin.
     * Only files with those mimetypes will be provided to the plugin via
     * the extract function.
     *
     * This can also contains partial mimetypes like "text/", in that case
     * this plugin will be chosen only if a better plugin does not exist.
     *
     * \return A StringList containing the mimetypes.
     * \sa extract
     */
    virtual QStringList mimetypes() const = 0;

    /**
     * The main function of the plugin that is responsible for extracting
     * the data and filling up the ExtractionResult
     *
     * The \p result provides the input url and mimetype which
     * can be used to identify the file.
     *
     * This function is synchronous and should be reentrant as it
     * can be called by multiple threads.
     */
    virtual void extract(ExtractionResult* result) = 0;

    //
    // Helper functions
    //

    /**
     * Tries to extract a valid date time from the string provided.
     */
    static QDateTime dateTimeFromString(const QString& dateString);

    /**
     * Tries to split the string into names. It cleans up any superfluous words
     * and removes extra junk such as curly braces
     */
//    static QStringList contactsFromString(const QString& string);

protected:
    /**
     * Return the inherited mimetype which the extractor directly supports.
     *
     * The returned type is one of the types from \c mimetypes(),
     * and is one of the ancestors of the input \p mimetype
     * (including \p mimetype itself).
     *
     * In case the mimetype is not a subtype of the supported types,
     * an empty QString() is returned.
     *
     */
    QString getSupportedMimeType(const QString& mimetype) const;

private:
    class ExtractorPluginPrivate;
    ExtractorPluginPrivate *d; // Placeholder for future binary compatible extensions
};
}

Q_DECLARE_INTERFACE(LingmoUIFileMetadata::ExtractorPlugin, "org.lingmo.lingmofilemetadata.ExtractorPlugin")

#endif // _LINGMOFILEMETADATA_EXTRACTOR_PLUGIN_H
