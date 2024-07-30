/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
    SPDX-FileCopyrightText: 2016 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "writedata.h"

#include <QMimeDatabase>

using namespace KFileMetaData;

class KFileMetaData::WriteDataPrivate
{
public:
    QString url;
    QString mimetype;
    PropertyMultiMap properties;
    QMap<EmbeddedImageData::ImageType, QByteArray> images;
};

WriteData::WriteData(const QString& url, const QString& mimetype)
    : d_ptr(new WriteDataPrivate)
{
    Q_D(WriteData);

    d->url = url;
    d->mimetype = mimetype;
    if (mimetype.isEmpty()) {
        d->mimetype = QMimeDatabase().mimeTypeForFile(url).name();
    }
}

WriteData::WriteData(const WriteData& rhs)
    : d_ptr(new WriteDataPrivate(*rhs.d_ptr))
{
}

WriteData& WriteData::operator=(const WriteData& rhs)
{
    *d_ptr = *rhs.d_ptr;
    return *this;
}

bool WriteData::operator==(const WriteData& rhs) const
{
    Q_D(const WriteData);
    return d->properties == rhs.d_ptr->properties;
}

void WriteData::add(Property::Property property, const QVariant& value)
{
    Q_D(WriteData);
    d->properties.insert(property, value);
}

void WriteData::addImageData(const QMap<EmbeddedImageData::ImageType, QByteArray>& images)
{
    Q_D(WriteData);
    d->images = images;
}

QMap<EmbeddedImageData::ImageType, QByteArray> WriteData::imageData() const
{
    Q_D(const WriteData);
    return d->images;
}

WriteData::~WriteData() = default;

QString WriteData::inputUrl() const
{
    Q_D(const WriteData);
    return d->url;
}

QString WriteData::inputMimetype() const
{
    Q_D(const WriteData);
    return d->mimetype;
}

PropertyMultiMap WriteData::properties() const
{
    Q_D(const WriteData);
    return d->properties;
}
