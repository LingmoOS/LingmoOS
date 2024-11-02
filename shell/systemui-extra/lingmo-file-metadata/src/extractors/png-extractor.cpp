/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>
    SPDX-FileCopyrightText: 2023 Junjie Bai <baijunjie@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "png-extractor.h"
#include "ocr-utils.h"
#include "thumbnail-utils.h"
#include <QImageReader>

using namespace LingmoUIFileMetadata;

static const QString PLUGIN_NAME = "Png";
static const QString VERSION = "1.0";

// Specified in https://www.w3.org/TR/PNG/#11keywords
const QMap<QString, Property::Property> keywordMap = {
        {QStringLiteral("Title"), Property::Title},
        {QStringLiteral("Author"), Property::Author},
        // QImage puts all text keys with spaces into the "Description" key，
        // (cf. qt_getImageTextFromDescription), overriding the actual PNG description, and making it useless.
        //{QStringLiteral("Description"), Property::Description},
        {QStringLiteral("Copyright"), Property::Copyright},
//        {QStringLiteral("Creation Time"), Property::CreationDate},
        {QStringLiteral("Software"), Property::Generator},
        {QStringLiteral("Comment"), Property::Comment}
        /**
         * other keywords:
         * Disclaimer - Legal disclaimer
         * Warning - Warning of nature of content
         * Source - Device used to create the image
        */
};

void PngExtractor::extract(ExtractionResult *result) {
    result->addType(Type::Image);

    if (result->inputFlags().testFlag(ExtractionResult::ExtractPlainText)) {
       result->append(OcrUtils::getTextInPicture(result->inputUrl()));
    }

    if (result->inputFlags() & ExtractionResult::ExtractThumbnail) {
        if (ThumbnailUtils::needGenerateThumbnail(result, PLUGIN_NAME, VERSION)) {
            QImage thumbnail(result->inputUrl());
            ThumbnailUtils::setThumbnail(result, thumbnail, PLUGIN_NAME, VERSION);
        }
    }

    if (result->inputFlags().testFlag(ExtractionResult::ExtractMetaData)) {
        QImageReader reader(result->inputUrl(), "png");
        if (!reader.canRead()) {
            return;
        }

        for (auto it = keywordMap.constBegin(); it != keywordMap.constEnd(); it++) {
            QString text = reader.text(it.key());
            /**
             * As the spec says:
             * Keywords shall be spelled exactly as registered, so that decoders can use simple literal comparisons when looking for particular keywords.
             * In particular, keywords are considered case-sensitive. Keywords are restricted to 1 to 79 bytes in length.
             * But in fact it may be different.
             */
            if (text.isEmpty()) {
                text = reader.text(it.key().toLower());
            }
            if(text.isEmpty()) {
                continue;
            }

            // Spec: the date format SHOULD be in the RFC 3339 date-time format or in the date format defined in section 5.2.14 of RFC 1123.
//            if (it.value() == Property::CreationDate) {
//                const QDateTime creationDate = QDateTime::fromString(text, Qt::RFC2822Date);
//                if (creationDate.isValid()) {
//                    result->add(it.value(), creationDate);
//                }
//            } else {
                result->add(it.value(), text);
//            }
        }
    }

}

PngExtractor::PngExtractor(QObject *parent) : ExtractorPlugin(parent) {

}

QStringList PngExtractor::mimetypes() const {
    return {
            QStringLiteral("image/png")
    };
}
