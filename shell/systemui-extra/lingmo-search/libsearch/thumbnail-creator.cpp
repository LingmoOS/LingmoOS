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

#include "thumbnail-creator.h"
#include "file-reader.h"

using namespace LingmoUISearch;

ThumbnailCreator::ThumbnailCreator(const QString &url, const QSize &targetSize, qreal dpr, QObject *parent): QObject(parent)
{
    setAutoDelete(true);
    m_url = url;
    m_targetSize = targetSize;
    m_dpr = dpr;
}

void ThumbnailCreator::run()
{
    Q_EMIT ready(m_url, FileReader::getInstance()->getThumbnail(m_url, m_targetSize, m_dpr));
}