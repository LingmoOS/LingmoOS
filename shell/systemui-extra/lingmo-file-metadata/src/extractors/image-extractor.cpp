/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijunjie <baijunjie@kylinos.cn>
 * Modified by: baijunjie <baijunjie@kylinos.cn>
 *
 */

#include "image-extractor.h"
#include "thumbnail-utils.h"
#include "ocr-utils.h"
#include <QImage>

using namespace LingmoUIFileMetadata;

ImageExtractor::ImageExtractor(QObject *parent) : ExtractorPlugin(parent) {}

static const QString VERSION = "1.0";
static const QString PLUGIN_NAME = "Image";

const QStringList supportedMimeTypes {
        QStringLiteral("image/png"),
        QStringLiteral("image/jpeg"),
        QStringLiteral("image/bmp"),
        QStringLiteral("image/tiff"),
        QStringLiteral("image/webp"),
        QStringLiteral("image/gif"),
        QStringLiteral("image/x-adobe-dng"),//DNG, can not ocr ,but can generate thumbnail
        QStringLiteral("image/svg+xml"),//svg, can not ocr
        QStringLiteral("image/x-portable-bitmap"),//pbm
        QStringLiteral("image/x-portable-graymap"),//pgm
        QStringLiteral("image/x-portable-pixmap"),//pnm ppm
        QStringLiteral("image/x-xpixmap"),//xpm, can not ocr
};

QStringList ImageExtractor::mimetypes() const {
    return supportedMimeTypes;
}

void ImageExtractor::extract(ExtractionResult *result) {
    if (!supportedMimeTypes.contains(result->inputMimetype()) ) {
        return;
    }

    result->addType(Type::Image);

    if (result->inputFlags() & ExtractionResult::ExtractThumbnail) {
        if (ThumbnailUtils::needGenerateThumbnail(result, PLUGIN_NAME, VERSION)) {
            QImage thumbnail(result->inputUrl());
            ThumbnailUtils::setThumbnail(result, thumbnail, PLUGIN_NAME, VERSION);
        }
    }

    if (result->inputFlags() & ExtractionResult::ExtractPlainText) {
        result->append(OcrUtils::getTextInPicture(result->inputUrl()));
    }
}
