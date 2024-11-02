/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: MeihongHe <hemeihong@kylinos.cn>
 *
 */

#include "image-pdf-thumbnail.h"
#include "generic-thumbnailer.h"
#include "file-utils.h"
#include <QFileInfo>
#include <QDebug>
#include <QtConcurrent>
#include <QImage>
#include <QMessageAuthenticationCode>
#include <QPainter>
#include <QImageReader>
#include <qglobal.h>

ImagePdfThumbnail::ImagePdfThumbnail(const QString &uri)
{
    if (!uri.startsWith("file:///")) {
        m_url = FileUtils::getTargetUri(uri);
        qDebug()<<"target uri:"<< m_url.path();
    }
    else {
        m_url = uri;
    }

    auto fileInfo = FileInfo::fromUri(uri);
    m_modifyTime = fileInfo->modifiedTime();
}

ImagePdfThumbnail::~ImagePdfThumbnail()
{

}

QIcon ImagePdfThumbnail::generateThumbnail()
{
    //FIXME need support to preview
    QIcon thumbnailImage = QIcon::fromTheme("atril");
    return thumbnailImage;
}

