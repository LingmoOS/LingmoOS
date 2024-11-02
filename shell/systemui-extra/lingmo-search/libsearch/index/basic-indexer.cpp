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
#include "basic-indexer.h"
#include <QDateTime>
#include <QFileInfo>
#include <QUrl>
#include <QTextBoundaryFinder>
#include "file-utils.h"
using namespace LingmoUISearch;
BasicIndexer::BasicIndexer(const QString& filePath): m_filePath(filePath)
{
}

bool BasicIndexer::index()
{
    QFileInfo info = QFileInfo(m_filePath);
    if(!info.exists()) {
        return false;
    }
    //添加数据
    m_document.setData(m_filePath);
    //唯一term
    m_document.setUniqueTerm(FileUtils::makeDocUterm(m_filePath));
    //上层文件夹term，用于在上层文件夹删除时删除自己
    m_document.addTerm("PARENTTERM" + FileUtils::makeDocUterm(m_filePath.section("/", 0, -2, QString::SectionIncludeLeadingSep)));
    //1-目录， 0-文件
    m_document.addValue(1, QString((info.isDir() && (!info.isSymLink())) ? "1" : "0"));
    //修改时间
    QString time = info.lastModified().toString("yyyyMMddHHmmsszzz");
    m_document.setIndexTime(time);
    m_document.addSortableSerialiseValue(2, time);

    QString indexName = info.fileName().toLower();
    QStringList pinyinTextList = FileUtils::findMultiToneWords(info.fileName());
    int i = 0;
    int postingCount = 1; //terms post of Xapian document is started from 1!
    int start = 0;
    QTextBoundaryFinder bf(QTextBoundaryFinder::Grapheme, indexName);
    for(; bf.position() != -1; bf.toNextBoundary()) {
        int end = bf.position();
        if(bf.boundaryReasons() & QTextBoundaryFinder::EndOfItem) {
            m_document.addPosting(QUrl::toPercentEncoding(indexName.mid(start, end - start)).toStdString(), postingCount);
            ++postingCount;
        }
        start = end;
    }

    for(QString& s : pinyinTextList) {
        i = 0;
        while(i < s.size()) {
            m_document.addPosting(QUrl::toPercentEncoding(s.at(i)).toStdString(), postingCount);
            ++postingCount;
            ++i;
        }
    }
    return true;
}
