/*
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "extractionresult.h"

using namespace KFileMetaData;

class KFileMetaData::ExtractionResultPrivate
{
public:
    QString url;
    QString mimetype;
    ExtractionResult::Flags flags;
    QMap<EmbeddedImageData::ImageType, QByteArray> images;
};

ExtractionResult::ExtractionResult(const QString& url, const QString& mimetype, const Flags& flags)
    : d(new ExtractionResultPrivate)
{
    d->url = url;
    d->mimetype = mimetype;
    d->flags = flags;
}

ExtractionResult::ExtractionResult(const ExtractionResult& rhs)
    : d(new ExtractionResultPrivate(*rhs.d))
{
}

ExtractionResult::~ExtractionResult() = default;

QString ExtractionResult::inputUrl() const
{
    return d->url;
}

QString ExtractionResult::inputMimetype() const
{
    return d->mimetype;
}

ExtractionResult::Flags ExtractionResult::inputFlags() const
{
    return d->flags;
}

void ExtractionResult::addImageData(QMap<EmbeddedImageData::ImageType, QByteArray>&& images)
{
    d->images = images;
}

QMap<EmbeddedImageData::ImageType, QByteArray>
ExtractionResult::imageData() const
{
    return d->images;
}

