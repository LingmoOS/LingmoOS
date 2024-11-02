/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: zhangpengfei <zhangpengfei@kylinos.cn>
 *
 */
#ifndef FILEREADER_H
#define FILEREADER_H

#include <extractor-manager.h>
#include <QImage>
namespace LingmoUISearch {
class FileReader{
public:
    static FileReader* getInstance();
    ~FileReader() = default;
    void getTextContent(const QString &path, QString &textContent, const QString &suffix);
    QImage getThumbnail(const QString &path, const QSize &targetSize, qreal dpr);

private:
    FileReader();
    LingmoUIFileMetadata::ExtractorManager m_extractorManager;

};
}
#endif // FILEREADER_H
