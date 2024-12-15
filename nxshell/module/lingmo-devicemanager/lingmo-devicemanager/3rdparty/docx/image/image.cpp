// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "image.h"

#include <QMimeDatabase>
#include <QMimeType>
#include <QBuffer>
#include <QImageReader>
#include <QLoggingCategory>

namespace Docx {

Image::Image(const PackURI &path)
{
    m_img = QImage(path);
    m_imgSize = m_img.size();


    QMimeDatabase base;
    QMimeType fileInfo = base.mimeTypeForFile(path);
    m_horzDpi = m_img.logicalDpiX();
    m_vertDpi = m_img.logicalDpiY();
    m_ext = fileInfo.preferredSuffix();
    m_contentType = fileInfo.name();
    //saveBlob();

}

Image::Image(const QByteArray &data, const QString &format)
{
    m_img.loadFromData(data, format.toStdString().c_str());
    m_imgSize = m_img.size();

    QMimeDatabase base;
    QMimeType fileInfo = base.mimeTypeForData(data);
    m_horzDpi = m_img.logicalDpiX();
    m_vertDpi = m_img.logicalDpiY();
    m_ext = fileInfo.preferredSuffix();
    m_contentType = fileInfo.name();
    m_blob = data;
}

QImage Image::img() const
{
    return m_img;
}

int Image::pxWidth() const
{
    return m_imgSize.width();
}

int Image::pxHeight() const
{
    return m_imgSize.height();
}

int Image::horzDpi() const
{
    return m_horzDpi;
}

int Image::vertDpi() const
{
    return m_vertDpi;
}

QString Image::ext() const
{
    return m_ext;
}

QString Image::contentType() const
{
    return m_contentType;
}

QByteArray Image::blob()
{
    if (m_blob.isEmpty())
        saveBlob();
    return m_blob;
}

Image::~Image()
{

}

void Image::saveBlob()
{
    QBuffer buffer(&m_blob);
    buffer.open(QIODevice::WriteOnly);
    m_img.save(&buffer, m_ext.toStdString().c_str());

}

}
