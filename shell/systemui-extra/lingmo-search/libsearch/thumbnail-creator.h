/*
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */

#ifndef LINGMO_SEARCH_THUMBNAILCREATOR_H
#define LINGMO_SEARCH_THUMBNAILCREATOR_H

#include <QObject>
#include <QRunnable>
#include <QSize>

namespace LingmoUISearch {
class ThumbnailCreator : public QObject, public QRunnable
{
Q_OBJECT
public:
    explicit ThumbnailCreator(const QString &url, const QSize &targetSize, qreal dpr, QObject *parent = nullptr);

protected:
    void run() override;

Q_SIGNALS:
    void ready(QString url, const QImage &image);

private:
    QString m_url;
    QSize m_targetSize;
    qreal m_dpr;
};
}


#endif //LINGMO_SEARCH_THUMBNAILCREATOR_H
