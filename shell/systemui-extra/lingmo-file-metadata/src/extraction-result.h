/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LINGMOFILEMETADATA_EXTRACTIONRESULT_H
#define LINGMOFILEMETADATA_EXTRACTIONRESULT_H

#include <QString>
#include <QVariant>

#include <memory>

#include "lingmo-file-metadata_global.h"
#include "embedded-image-data.h"
#include "properties.h"
#include "types.h"
#include "thumbnail.h"

namespace LingmoUIFileMetadata {
class ExtractionResultPrivate;
/**
 * \class ExtractionResult extraction-result.h
 *
 * \brief The ExtractionResult class is where all the data extracted by
 * the indexer is saved. This class acts as a base class which should be
 * derived from and then passed to the relevant plugins.
 *
 * The derived class needs to implement 3 pure virtual functions through
 * which it receives the extracted data.
 *
 * \author Vishesh Handa <me@vhanda.in>
 */
class LINGMOFILEMETADATA_EXPORT ExtractionResult
{
public:
    /**
     * @see Flags
     */
    enum Flag {
        ExtractNothing = 0,
        ExtractMetaData = 1,
        ExtractPlainText = 2,
        ExtractImageData = 4,
        ExtractThumbnail = 8
    };
    /**
     * Stores a combination of #Flag values.
     */
    Q_DECLARE_FLAGS(Flags, Flag)

    /**
     * Create an ExtractionResult which can be passed be to Extractors. The
     * extractors use the \p url, \p mimetype and \p flags in order to determine
     * which file the data should be extracted from and which data should
     * be extracted.
     */
    explicit ExtractionResult(const QString& url, const QString& mimetype = QString(), const Flags& flags = Flags{ExtractPlainText | ExtractMetaData});
    ExtractionResult(const ExtractionResult& rhs);
    virtual ~ExtractionResult();

    /**
     * The input url which the plugins will use to locate the file
     */
    QString inputUrl() const;

    /**
     * The input mimetype. This mimetype should correspond with the
     * mimetypes supported with the relevant plugin when it is being
     * passed to the Extractor, or be a subtype thereof.
     *
     * \sa ExtractorManager::fetchExtractors
     * \sa ExtractorPlugin::supportedMimeType
     */
    QString inputMimetype() const;

    /**
     * The flags which the extraction plugin should considering following
     * when extracting metadata from the file
     */
    Flags inputFlags() const;

    /**
     * This function is called by plugins when they wish for some plain
     * text to be indexed without any property. This generally corresponds
     * to the text content in a file
     */
    virtual void append(const QString& text) = 0;

    /**
     * This function is called by the plugins when they wish to
     * add a key value pair which should be indexed. This function may be
     * called multiple times for the same key.
     *
     * \p property This specifies a property name. It should be one of the
     *             properties from the global list of properties.
     *
     * \p value The value of the property
     */
    virtual void add(Property::Property property, const QVariant& value) = 0;

    /**
     * This function is called by the plugins.
     * A type is a higher level classification of the file. A file can
     * have multiple types, but mostly when it does, those types are related.
     * Eg - Document and Presentation.
     *
     * Please choose one type from the list of available types
     */
    virtual void addType(Type::Type type) = 0;

    /**
     * This function is called by the plugins.
     *
     * \p images The images to add
     * \sa EmbeddedImageData
     */
    void addImageData(QMap<EmbeddedImageData::ImageType, QByteArray>&& images);

    /**
     * Return embedded image data
     *
     * \sa Flags::ExtractImageData
     */
    QMap<EmbeddedImageData::ImageType, QByteArray> imageData() const;

    /**
     * This function is called fot adding request for thumbnail extraction.
     * @param request
     */
    void setThumbnailRequest(const ThumbnailRequest &request);
    /**
     * This function is called by the plugins.
     * @return
     */
    const ThumbnailRequest &thumbnailRequest() const;
    /**
     * This function is called by the plugins.
     * \p thumbnail The thumbnail to add
     */
    void addThumbnail(const Thumbnail &thumbnail);
    /**
     * Return the thumbnail.
     * @return
     */
    Thumbnail thumbnail() const;

private:
    const std::unique_ptr<ExtractionResultPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ExtractionResult::Flags)

}

#endif // LINGMOFILEMETADATA_EXTRACTIONRESULT_H
