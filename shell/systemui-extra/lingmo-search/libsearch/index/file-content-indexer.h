/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 *
 */
#ifndef FILECONTENTINDEXER_H
#define FILECONTENTINDEXER_H

#include "document.h"
namespace LingmoUISearch {
class fileContentIndexer
{
public:
    fileContentIndexer(const QString& filePath);
    bool index();
    Document document() { return m_document; }

private:
    QString m_filePath;
    Document m_document;
};
}
#endif // FILECONTENTINDEXER_H
