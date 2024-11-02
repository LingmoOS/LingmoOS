/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "extraction-result.h"

using namespace LingmoUIFileMetadata;

class LingmoUIFileMetadata::ExtractionResultPrivate
{
public:
    QString m_url;
    QString m_mimetype;
    ExtractionResult::Flags m_flags;
    QMap<EmbeddedImageData::ImageType, QByteArray> m_images;
    ThumbnailRequest m_thumbnailRequest;
    Thumbnail m_thumbnail;
};

ExtractionResult::ExtractionResult(const QString& url, const QString& mimetype, const Flags& flags)
    : d(new ExtractionResultPrivate)
{
    d->m_url = url;
    d->m_mimetype = mimetype;
    d->m_flags = flags;
}

ExtractionResult::ExtractionResult(const ExtractionResult& rhs)
    : d(new ExtractionResultPrivate(*rhs.d))
{
}

ExtractionResult::~ExtractionResult() = default;

QString ExtractionResult::inputUrl() const
{
    return d->m_url;
}

QString ExtractionResult::inputMimetype() const
{
    return d->m_mimetype;
}

ExtractionResult::Flags ExtractionResult::inputFlags() const
{
    return d->m_flags;
}

void ExtractionResult::addImageData(QMap<EmbeddedImageData::ImageType, QByteArray>&& images)
{
    d->m_images = images;
}

QMap<EmbeddedImageData::ImageType, QByteArray>
ExtractionResult::imageData() const
{
    return d->m_images;
}

void ExtractionResult::setThumbnailRequest(const ThumbnailRequest &request)
{
    d->m_thumbnailRequest = request;
}

const ThumbnailRequest &ExtractionResult::thumbnailRequest() const
{
    return d->m_thumbnailRequest;
}

void ExtractionResult::addThumbnail(const Thumbnail &thumbnail)
{
    d->m_thumbnail = thumbnail;
}

Thumbnail ExtractionResult::thumbnail() const
{
    return d->m_thumbnail;
}
