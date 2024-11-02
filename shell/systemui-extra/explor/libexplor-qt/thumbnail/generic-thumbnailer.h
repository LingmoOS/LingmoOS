/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef GENERICTHUMBNAILER_H
#define GENERICTHUMBNAILER_H

#include <QObject>
#include <QSize>

class GenericThumbnailer : public QObject
{
    Q_OBJECT
public:
    static QIcon generateThumbnail(const QUrl &url, bool shadow = false, const QSize &size = QSize());
    static QIcon generateThumbnail(const QString &path, bool shadow = false, const QSize &size = QSize());
    static QIcon generateThumbnail(const QPixmap &pixmap, bool shadow = true, const QSize &size = QSize());
    static QString codeMd5(QString fileName);
    static QString codeMd5WithModifyTime(QString fileName, quint64 &modifyTime);
    static QString cachDir();
    static QString thumbnaileCachDir();
private:
    explicit GenericThumbnailer(QObject *parent = nullptr);
};

#endif // GENERICTHUMBNAILER_H
