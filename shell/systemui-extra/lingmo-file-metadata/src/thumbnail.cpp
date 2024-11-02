/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include "thumbnail.h"
#include <QImage>

using namespace LingmoUIFileMetadata;
class ThumbnailRequestPrivate
{
public:
    QSize m_targetSize = QSize(128, 128);
    qreal m_dpr = 1;
};

ThumbnailRequest::ThumbnailRequest(): d(new ThumbnailRequestPrivate)
{
}

ThumbnailRequest::ThumbnailRequest(const QSize &targetSize, qreal dpr)
        : d(new ThumbnailRequestPrivate)
{
    d->m_targetSize = targetSize;
    d->m_dpr = dpr;
}
ThumbnailRequest::ThumbnailRequest(const ThumbnailRequest &other): d(new ThumbnailRequestPrivate(*other.d))
{
}
ThumbnailRequest &ThumbnailRequest::operator=(const ThumbnailRequest &other)
{
    *d = *other.d;
    return *this;
}
ThumbnailRequest &ThumbnailRequest::operator=(ThumbnailRequest &&other) noexcept
{
    d = other.d;
    other.d = nullptr;
    return *this;
}

ThumbnailRequest::~ThumbnailRequest()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

QSize ThumbnailRequest::targetSize() const
{
    return d->m_targetSize;
}

qreal ThumbnailRequest::devicePixelRatio() const
{
    return d->m_dpr;
}

class ThumbnailPrivate
{
public:
    QImage m_image;
};

Thumbnail::Thumbnail(const QImage &image): d(new ThumbnailPrivate)
{
    d->m_image = image;
}
Thumbnail::Thumbnail(): d(new ThumbnailPrivate)
{
}

Thumbnail::Thumbnail(const Thumbnail &other): d(new ThumbnailPrivate(*other.d))
{
}

Thumbnail &Thumbnail::operator=(const Thumbnail &other)
{
    *d = *other.d;
    return *this;
}

Thumbnail::~Thumbnail()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

QImage Thumbnail::image() const {
    return d->m_image;
}

bool Thumbnail::isValid() const {
    return !d->m_image.isNull();
}

Thumbnail &Thumbnail::operator=(Thumbnail &&other) noexcept
{
    d = other.d;
    other.d = nullptr;
    return *this;
}