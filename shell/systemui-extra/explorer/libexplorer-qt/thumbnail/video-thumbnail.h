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
 * Authors: renpeijia <renpeijia@kylinos.cn>
 *
 */

#ifndef VIDEOTHUMBNAIL_H
#define VIDEOTHUMBNAIL_H

#include "file-info.h"
#include <QHash>
#include <QIcon>
#include <QMutex>
#include <QUrl>

using namespace Peony;

class VideoThumbnail{
public:
    explicit VideoThumbnail(const QString &uri);
    ~VideoThumbnail();
    QIcon generateThumbnail();

private:
    QMap<QString, QString> videoInfo();
    QUrl m_url;
    quint64 m_modifyTime = 0;
};

#endif // VIDEOTHUMBNAIL_H
