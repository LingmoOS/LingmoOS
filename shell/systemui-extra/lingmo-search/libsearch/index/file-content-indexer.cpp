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
#include "file-content-indexer.h"
#include <QDateTime>
#include <QFileInfo>
#include "file-reader.h"
#include "file-utils.h"
#include "chinese-segmentation.h"

using namespace LingmoUISearch;
fileContentIndexer::fileContentIndexer(const QString &filePath): m_filePath(filePath)
{
}

bool fileContentIndexer::index()
{
    QString content;
    QFileInfo info(m_filePath);
    if(!info.exists()) {
        return false;
    }
    QString suffix = info.suffix();
    FileReader::getInstance()->getTextContent(m_filePath, content, suffix);
    if(content.isEmpty()) {
        return false;
    }

    m_document.setData(content);

    std::vector<KeyWord> term = ChineseSegmentation::getInstance()->callSegment(content);
    content.clear();
    content.squeeze();

    for(auto & i : term) {
        m_document.addPosting(i.word, i.offsets, static_cast<int>(i.weight));
    }
    term.clear();
    term.shrink_to_fit();

    m_document.setUniqueTerm(FileUtils::makeDocUterm(m_filePath));
    m_document.addTerm("PARENTTERM" + FileUtils::makeDocUterm(m_filePath.section("/", 0, -2, QString::SectionIncludeLeadingSep)));
    m_document.addValue(1, m_filePath);
    m_document.addValue(2, suffix);

    QString time = info.lastModified().toString("yyyyMMddHHmmsszzz");
    m_document.addSortableSerialiseValue(3, time);
    m_document.setIndexTime(time);

    return true;
}
