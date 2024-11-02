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

#ifndef IMAGEPDFTHUMBNAIL_H
#define IMAGEPDFTHUMBNAIL_H

#include "file-info.h"
#include <QHash>
#include <QIcon>
#include <QMutex>
#include <QUrl>

using namespace Peony;

class ImagePdfThumbnail{
public:
    explicit ImagePdfThumbnail(const QString &uri);
    ~ImagePdfThumbnail();
    QIcon generateThumbnail();

private:
    /*
    * 提供djuv文件首页转换为图片的存储路径
    */
    void thumbnaileCachDir();

    QUrl m_url;
    /*
    * 获取文件的修改时间，如果被修改，将重新生成缩略图，
    * 主要是为了处理修改文件首页的情况
    */
    quint64 m_modifyTime = 0;
};

#endif // IMAGEPDFTHUMBNAIL_H
