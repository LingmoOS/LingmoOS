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
#include "search-manager.h"

#include <utility>
#include "dir-watcher.h"
#include "file-indexer-config.h"

using namespace LingmoUISearch;

size_t SearchManager::uniqueSymbolFile = 0;
size_t SearchManager::uniqueSymbolDir = 0;
size_t SearchManager::uniqueSymbolContent = 0;
QMutex  SearchManager::m_mutexFile;
QMutex  SearchManager::m_mutexDir;
QMutex  SearchManager::m_mutexContent;

SearchManager::SearchManager(QObject *parent) : QObject(parent) {
}

SearchManager::~SearchManager() {
}

uint SearchManager::getCurrentIndexCount() {
    try {
        Xapian::Database db(INDEX_PATH);
        return db.get_doccount();
    } catch(const Xapian::Error &e) {
        qWarning() << QString::fromStdString(e.get_description());
        return 0;
    }
}

bool SearchManager::isBlocked(QString &path) {
    QStringList blockList = DirWatcher::getDirWatcher()->getBlockDirsOfUser();
    for(const QString& i : blockList) {
        if(FileUtils::isOrUnder(path, i))
            return true;
    }
    return false;

}

bool SearchManager::creatResultInfo(SearchPluginIface::ResultInfo &ri, const QString& path)
{
    QFileInfo info(path);
    if(!info.exists()) {
        return false;
    }
    ri.icon = FileUtils::getFileIcon(QUrl::fromLocalFile(path).toString(), false);
    ri.name = info.fileName().replace("\r", " ").replace("\n", " ");
    ri.toolTip = info.fileName();
    ri.resourceType = QStringLiteral("file");
    ri.description = QVector<SearchPluginIface::DescriptionInfo>() \
                    << SearchPluginIface::DescriptionInfo{tr("Path:"), path} \
                    << SearchPluginIface::DescriptionInfo{tr("Modified time:"), info.lastModified().toString("yyyy/MM/dd hh:mm:ss")};
    ri.actionKey = path;
    if (FileIndexerConfig::getInstance()->ocrContentIndexTarget()[info.suffix()]) {
        ri.type = 1;//1为ocr图片文件
    } else {
        ri.type = 0;//0为默认文本文件
    }
    return true;
}

FileSearch::FileSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, size_t uniqueSymbol, QString keyword, QString value, unsigned slot, int begin, int num) {
    this->setAutoDelete(true);
    m_search_result = searchResult;
    m_uniqueSymbol = uniqueSymbol;
    m_keyword = std::move(keyword);
    m_value = std::move(value);
    m_slot = slot;
    m_begin = begin;
    m_num = num;
    m_matchDecider = new FileMatchDecider();
}

FileSearch::~FileSearch() {
    m_search_result = nullptr;
    if(m_matchDecider)
        delete m_matchDecider;
}

void FileSearch::run() {
    if(m_value == "0") {
        SearchManager::m_mutexFile.lock();
        if(!m_search_result->isEmpty()) {
            m_search_result->clear();
        }
        SearchManager::m_mutexFile.unlock();
    } else if(m_value == "1") {
        SearchManager::m_mutexDir.lock();
        if(!m_search_result->isEmpty()) {
            m_search_result->clear();
        }
        SearchManager::m_mutexDir.unlock();
    }
    //目前的需求是文件搜索数量无上限。
    //但如果不设置单次搜索数量限制，在一些性能非常弱的机器上（如兆芯某些机器），就算我们这里不阻塞UI，也会因为搜索本身占用cpu过多（可能）导致UI卡顿。
    //可能会有更好的方法，待优化。
    m_begin = 0;
    m_num = 100;
    uint resultCount = 1;
    uint totalCount = 0;
    while(resultCount > 0) {
        resultCount = keywordSearchFile();
        m_begin += m_num;
        totalCount += resultCount;
    }
    qDebug() << "Total count:" << m_value << totalCount;
}

uint FileSearch::keywordSearchFile() {
    try {
        qDebug() << "--keywordSearchFile start--";
        Xapian::Database db(INDEX_PATH);
        Xapian::Query query = creatQueryForFileSearch();
        Xapian::Enquire enquire(db);

        Xapian::Query queryFile;
        if(!m_value.isEmpty()) {
            std::string slotValue = m_value.toStdString();
            Xapian::Query queryValue = Xapian::Query(Xapian::Query::OP_VALUE_RANGE, m_slot, slotValue, slotValue);
            queryFile = Xapian::Query(Xapian::Query::OP_AND, query, queryValue);
        } else {
            queryFile = query;
        }

        qDebug() << "keywordSearchFile:" << QString::fromStdString(queryFile.get_description());

        enquire.set_query(queryFile);
        enquire.set_docid_order(Xapian::Enquire::DONT_CARE);
        enquire.set_sort_by_relevance_then_value(2, true);
        Xapian::MSet result = enquire.get_mset(m_begin, m_num, nullptr, m_matchDecider);
        uint resultCount = result.size();
        qDebug() << "keywordSearchFile results count=" << resultCount;
        if(resultCount == 0)
            return 0;
        if(getResult(result) == -1)
            return 0;

        qDebug() << "--keywordSearchFile finish--";
        return resultCount;
    } catch(const Xapian::Error &e) {
        qWarning() << QString::fromStdString(e.get_description());
        qDebug() << "--keywordSearchFile finish--";
        return 0;
    }
}

Xapian::Query FileSearch::creatQueryForFileSearch() {
    auto userInput = m_keyword.toLower();
    std::vector<Xapian::Query> v;
    QTextBoundaryFinder bf(QTextBoundaryFinder::Grapheme, userInput);
    int start = 0;
    for(; bf.position() != -1; bf.toNextBoundary()) {
        int end = bf.position();
        if(bf.boundaryReasons() & QTextBoundaryFinder::EndOfItem) {
            v.emplace_back(QUrl::toPercentEncoding(userInput.mid(start, end - start)).toStdString());
        }
        start = end;
    }
    Xapian::Query queryPhrase = Xapian::Query(Xapian::Query::OP_PHRASE, v.begin(), v.end());
    return queryPhrase;
}

int FileSearch::getResult(Xapian::MSet &result) {
    for(auto it = result.begin(); it != result.end(); ++it) {
        Xapian::Document doc = it.get_document();
        std::string data = doc.get_data();
//        Xapian::weight docScoreWeight = it.get_weight();
//        Xapian::percent docScorePercent = it.get_percent();
//        std::string date = doc.get_value(2);

        QString path = QString::fromStdString(data);
        std::string().swap(data);

        SearchPluginIface::ResultInfo ri;
        if(SearchManager::creatResultInfo(ri, path)) {
            switch(m_value.toInt()) {
            case 1:
                SearchManager::m_mutexDir.lock();
                if(m_uniqueSymbol == SearchManager::uniqueSymbolDir) {
                    m_search_result->enqueue(ri);
                    SearchManager::m_mutexDir.unlock();
                } else {
                    SearchManager::m_mutexDir.unlock();
                    return -1;
                }

                break;
            case 0:
                SearchManager::m_mutexFile.lock();
                if(m_uniqueSymbol == SearchManager::uniqueSymbolFile) {
                    m_search_result->enqueue(ri);
                    SearchManager::m_mutexFile.unlock();
                } else {
                    SearchManager::m_mutexFile.unlock();
                    return -1;
                }
                break;
            default:
                break;
            }
        }
//        qDebug() << "doc=" << path
//                 << ",weight="
//                 << docScoreWeight
//                 << ",percent="
//                 << docScorePercent
//                 << "date"
//                 << QString::fromStdString(date);
    }
    //        if(!pathTobeDelete->isEmpty())
    //            deleteAllIndex(pathTobeDelete)
    return 0;
}

FileContentSearch::FileContentSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, size_t uniqueSymbol, QString keyword, bool fuzzy, int begin, int num)
    :m_search_result(searchResult),
     m_uniqueSymbol(uniqueSymbol),
     m_keyword(std::move(keyword)),
     m_fuzzy(fuzzy),
     m_begin(begin),
     m_num(num)
{
    this->setAutoDelete(true);
    m_matchDecider = new FileContentMatchDecider();
}

FileContentSearch::~FileContentSearch() {
    m_search_result = nullptr;
    if(m_matchDecider)
        delete m_matchDecider;
}

void FileContentSearch::run() {
    SearchManager::m_mutexContent.lock();
    if(!m_search_result->isEmpty()) {
        m_search_result->clear();
    }
    SearchManager::m_mutexContent.unlock();

    //这里同文件搜索，待优化。
    m_begin = 0;
    m_num = 100;
    uint resultCount = 1;
    uint totalCount = 0;
    while(resultCount > 0) {
        resultCount = keywordSearchContent();
        m_begin += m_num;
        totalCount += resultCount;
    }
    qDebug() << "Total count:" << totalCount;
}

uint FileContentSearch::keywordSearchContent() {
    try {
        qDebug() << "--keywordSearchContent search start--";

        Xapian::Database db(CONTENT_INDEX_PATH);
        if(FileIndexerConfig::getInstance()->isOCREnable()) {
            db.add_database(Xapian::Database(OCR_CONTENT_INDEX_PATH));
        }
        Xapian::Enquire enquire(db);
        Xapian::QueryParser qp;
        qp.set_default_op(Xapian::Query::OP_AND);
        qp.set_database(db);

        std::vector<KeyWord> sKeyWord = ChineseSegmentation::getInstance()->callSegment(m_keyword);
        //Creat a query
        std::string words;
        for(auto & i : sKeyWord) {
            words.append(i.word).append(" ");
        }

//        Xapian::Query query = qp.parse_query(words);

        std::vector<Xapian::Query> v;
        for(auto & i : sKeyWord) {
            v.emplace_back(i.word);
//            qDebug() << QString::fromStdString(sKeyWord.at(i).word);
        }
        Xapian::Query query;
        if(m_fuzzy) {
            query = Xapian::Query(Xapian::Query::OP_OR, v.begin(), v.end());
        } else {
            query = Xapian::Query(Xapian::Query::OP_AND, v.begin(), v.end());
        }

        qDebug() << "keywordSearchContent:" << QString::fromStdString(query.get_description());

        enquire.set_query(query);

        Xapian::MSet result = enquire.get_mset(m_begin, m_num, nullptr, m_matchDecider);
        uint resultCount = result.size();
        if(result.empty()) {
            return 0;
        }
        qDebug() << "keywordSearchContent results count=" << resultCount;

        if(getResult(result, words) == -1) {
            return 0;
        }

        qDebug() << "--keywordSearchContent search finish--";
        return resultCount;
    } catch(const Xapian::Error &e) {
        qWarning() << QString::fromStdString(e.get_description());
        qDebug() << "--keywordSearchContent search finish--";
        return 0;
    }
}

int FileContentSearch::getResult(Xapian::MSet &result, std::string &keyWord) {
    for(auto it = result.begin(); it != result.end(); ++it) {
        Xapian::Document doc = it.get_document();
        std::string data = doc.get_data();
//        double docScoreWeight = it.get_weight();
//        Xapian::percent docScorePercent = it.get_percent();
        QString path = QString::fromStdString(doc.get_value(1));
//        QString suffix = QString::fromStdString(doc.get_value(2));

        SearchPluginIface::ResultInfo ri;
        if(!SearchManager::creatResultInfo(ri, path)) {
            continue;
        }
        // Construct snippets containing keyword.
        auto termIterator = doc.termlist_begin();
        QStringList words = QString::fromStdString(keyWord).split(" ", Qt::SkipEmptyParts);

        for(const QString& wordTobeFound : words) {
            std::string term = wordTobeFound.toStdString();
            termIterator.skip_to(term);
            if(termIterator == doc.termlist_end()) {
                termIterator = doc.termlist_begin();
                continue;
            }
            if(term == *termIterator) {
                break;
            } else {
                termIterator = doc.termlist_begin();
            }
        }
        auto pos = termIterator.positionlist_begin();
        QString snippet = FileUtils::getSnippet(data, *pos, QString::fromStdString(keyWord).remove(" "));

        ri.description.prepend(SearchPluginIface::DescriptionInfo{"",FileUtils::getHtmlText(snippet, QString::fromStdString(keyWord).remove(" "))});
        QString().swap(snippet);
        std::string().swap(data);

        SearchManager::m_mutexContent.lock();
        if(m_uniqueSymbol == SearchManager::uniqueSymbolContent) {
            m_search_result->enqueue(ri);
            SearchManager::m_mutexContent.unlock();
        } else {
            SearchManager::m_mutexContent.unlock();
            return -1;
        }
        //qDebug() << "path=" << path << ",weight=" << docScoreWeight << ",percent=" << docScorePercent;
    }
    return 0;
}

DirectSearch::DirectSearch(QString keyword, DataQueue<SearchPluginIface::ResultInfo> *searchResult, QString value, size_t uniqueSymbol) {
    this->setAutoDelete(true);
    m_keyword = std::move(keyword);
    m_searchResult = searchResult;
    m_uniqueSymbol = uniqueSymbol;
    m_value = std::move(value);
}

void DirectSearch::run() {
    QStringList blockList = DirWatcher::getDirWatcher()->getBlockDirsOfUser();
    QStringList searchPath = DirWatcher::getDirWatcher()->currentIndexableDir();
    QQueue<QString> bfs;
    for (const QString &path : searchPath) {
        bool underBlock(false);
        for (const QString &blockDir : blockList) {
            if (FileUtils::isOrUnder(path, blockDir)) {
                underBlock = true;
                break;
            }
        }
        if (!underBlock) {
            blockList.append(DirWatcher::getDirWatcher()->blackListOfDir(path));
            bfs.enqueue(path);
            match(QFileInfo(path));
        }
    }
    if (bfs.isEmpty()) {
        return;
    }

    QFileInfoList list;
    QDir dir;
    // QDir::Hidden
    if(m_value == DIR_SEARCH_VALUE) {
         dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    } else {
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        dir.setSorting(QDir::DirsFirst);
    }
    while(!bfs.empty()) {
        dir.setPath(bfs.dequeue());
        list = dir.entryInfoList();
        for (auto i : list) {
            if (i.isDir() && (!(i.isSymLink()))) {
                bool findIndex = false;
                for (const QString& j : blockList) {
                    if (FileUtils::isOrUnder(i.absoluteFilePath(), j)) {
                        findIndex = true;
                        break;
                    }
                }
                if (findIndex) {
                    qDebug() << "path is blocked:" << i.absoluteFilePath();
                    continue;
                }
                bfs.enqueue(i.absoluteFilePath());
            }
            SearchManager::m_mutexDir.lock();
            if(m_uniqueSymbol == SearchManager::uniqueSymbolDir) {
                match(i);
                SearchManager::m_mutexDir.unlock();
            } else {
                SearchManager::m_mutexDir.unlock();
                return;
            }
        }
    }
}

void DirectSearch::match(const QFileInfo &info)
{
    if(info.fileName().contains(m_keyword, Qt::CaseInsensitive)) {
        if((info.isDir() && m_value == DIR_SEARCH_VALUE) || (info.isFile() && m_value == FILE_SEARCH_VALUE)) {
            SearchPluginIface::ResultInfo ri;
            if(SearchManager::creatResultInfo(ri,info.absoluteFilePath())) {
                m_searchResult->enqueue(ri);
            }
        }
    }
}

bool FileMatchDecider::operator ()(const Xapian::Document &doc) const
{
    QString path = QString::fromStdString(doc.get_data());
    if(SearchManager::isBlocked(path)) {
        return false;
    }
    return true;
}

bool FileContentMatchDecider::operator ()(const Xapian::Document &doc) const
{
    QString path = QString::fromStdString(doc.get_value(1));
    if(SearchManager::isBlocked(path)) {
        return false;
    }
    return true;
}