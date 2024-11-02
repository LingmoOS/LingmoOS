/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Burgess Chang <brs@sdf.org>
 *
 */

#include <QDebug>
#include <QImage>
#include <QStandardPaths>

#include "pdf-thumbnail.h"

PdfThumbnail::PdfThumbnail(const QString &url, unsigned int pageNum)
    : pageNum(pageNum), shortUrl(url) {
    shortUrl = shortUrl.remove("file://");
    documentPrivate = Poppler::Document::load(shortUrl);
    if (!documentPrivate || documentPrivate->isLocked()) {
        qDebug() << "load pdf documnet failed";
    }
}

PdfThumbnail::~PdfThumbnail() {
    delete documentPrivate;
    delete pagePrivate;
}

QPixmap PdfThumbnail::generateThumbnail(unsigned int pageNum) {
    try {
        if (this->documentPrivate == nullptr || this->documentPrivate->isLocked())
            //throw "pdf document not existed";
            //fix crash issue, change throw to return
            return QPixmap();
        pagePrivate = documentPrivate->page(pageNum);
        if (pagePrivate == nullptr)
            //throw "load pdf page failed";
            return QPixmap();
        auto image = pagePrivate->renderToImage(
                         72 * 2, 72 * 2, 0, 0, pagePrivate->pageSizeF().width() * 2,
                         pagePrivate->pageSizeF().height() * 2);
        if (image.isNull())
            //throw "load pdf page image failed";
            return QPixmap();
        return QPixmap::fromImage(image);
    } catch (char *e) {
        qDebug() << e;
        return QPixmap();
    }
}
