/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */
#include "file-search-task.h"
#include <QDir>
#include <QUrl>
#include <QFile>
#include <QQueue>
#include <QDebug>
#include <QDateTime>
#include <gio/gio.h>
#include <QUrl>
#include <QTextBoundaryFinder>
#include "index-status-recorder.h"
#include "dir-watcher.h"
#include "common.h"
#include "file-utils.h"

using namespace LingmoUISearch;
FileSearchTask::FileSearchTask(QObject *parent)
{
    this->setParent(parent);
    qRegisterMetaType<size_t>("size_t");
    m_pool = new QThreadPool(this);
    m_pool->setMaxThreadCount(1);
}

FileSearchTask::~FileSearchTask()
{
    m_pool->clear();
    m_pool->waitForDone();
}

void FileSearchTask::setController(const SearchController &searchController)
{
    m_searchController = searchController;
}

const QString FileSearchTask::name()
{
    return "File";
}

const QString FileSearchTask::description()
{
    return "File search.";
}

QString FileSearchTask::getCustomSearchType()
{
    return "File";
}

void FileSearchTask::startSearch()
{
    FileSearchWorker *fileSearchWorker;
    fileSearchWorker = new FileSearchWorker(this, &m_searchController);
    m_pool->start(fileSearchWorker);
}

void FileSearchTask::stop()
{

}

bool FileSearchTask::isSearching()
{
    return m_pool->activeThreadCount() > 0;
}

FileSearchWorker::FileSearchWorker(FileSearchTask *fileSarchTask, SearchController *searchController) : m_FileSearchTask(fileSarchTask)
{
    m_searchController = searchController;
    m_currentSearchId = m_searchController->getCurrentSearchId();
}

void FileSearchWorker::run()
{
    //1.检查是否为不可搜索目录
    QStringList searchDirs = m_searchController->getSearchDir();
    searchDirs.removeDuplicates();

    for (QString &dir : searchDirs) {
        if (QFileInfo::exists(dir)) {
            if (dir.endsWith("/") && dir != "/") {
                dir.remove(dir.length() - 1, 1);
            }

            QStringList blackListTmp = DirWatcher::getDirWatcher()->blackListOfDir(dir);
            if (!blackListTmp.contains(dir)) {
                m_validDirectories.append(dir);
                m_blackList.append(blackListTmp);
            }
        }
    }

    //过滤空标签
    for (QString &label : m_searchController->getFileLabel()) {
        if (!label.isEmpty()) {
            m_labels.append(label);
        }
    }

    bool finished = true;
    bool indexed = true;

    QStringList indexedDir = DirWatcher::getDirWatcher()->currentIndexableDir();
    if(!indexedDir.isEmpty() && IndexStatusRecorder::getInstance()->indexDatabaseEnable() && !m_searchController->searchHiddenFiles()) {
        for(const QString &path : m_searchController->getSearchDir()) {
            bool pathIndexed = false;
            for(const QString &dir : indexedDir) {
                if(FileUtils::isOrUnder(path, dir)) {
                    pathIndexed = true;
                    break;
                }
            }
            if(!pathIndexed) {
                indexed = false;
                break;
            }
        }
    } else {
        indexed = false;
    }

    //TODO 还需要判断是否为不能建立索引的目录
    if (indexed) {
        qDebug() << "index ready";
        finished = searchWithIndex();
    } else {
        if (m_validDirectories.empty()) {
            //TODO 使用全局的默认可搜索目录
            if (QFileInfo::exists(QDir::homePath())) {
                sendErrorMsg(QObject::tr("Warning, Can not find home path."));
                return;
            }
            m_validDirectories.append(QDir::homePath());
            m_blackList.append(DirWatcher::getDirWatcher()->blackListOfDir(QDir::homePath()));
        }
        qDebug() << "direct search";
        finished = directSearch();
    }

    if (finished) QMetaObject::invokeMethod(m_FileSearchTask, "searchFinished", Q_ARG(size_t, m_currentSearchId));
}

Xapian::Query FileSearchWorker::creatQueryForFileSearch() {
    Xapian::Query fileOrDir = Xapian::Query::MatchAll;
    if (m_searchController->isSearchDirOnly() && m_searchController->isSearchFileOnly()) {
        //同时指定'只搜索目录'和'只搜索文件',那麼拒絕搜索
        return {};

    } else {
        if (m_searchController->isSearchDirOnly()) {
            fileOrDir = Xapian::Query(Xapian::Query::OP_VALUE_RANGE, 1, "1", "1");

        } else if (m_searchController->isSearchFileOnly()) {
            fileOrDir = Xapian::Query(Xapian::Query::OP_VALUE_RANGE, 1, "0", "0");
        }
    }

    std::vector<Xapian::Query> queries;
    if (m_searchController->getKeyword().isEmpty() && m_searchController->getMatchAllIfNoKeyword()) {
        queries.emplace_back(Xapian::Query::MatchAll);
    } else {
        for (QString &keyword : m_searchController->getKeyword()) {
            if (!keyword.isEmpty()) {
                std::vector<Xapian::Query> queryOfKeyword;
                QString inputKeyWord = keyword.toLower();
                QTextBoundaryFinder bf(QTextBoundaryFinder::Grapheme, inputKeyWord.toLower());
                int start = 0;
                for(; bf.position() != -1; bf.toNextBoundary()) {
                    int end = bf.position();
                    if(bf.boundaryReasons() & QTextBoundaryFinder::EndOfItem) {
                        queryOfKeyword.emplace_back(QUrl::toPercentEncoding(inputKeyWord.toLower().mid(start, end - start)).toStdString());
                    }
                    start = end;
                }
                queries.emplace_back(Xapian::Query::OP_PHRASE, queryOfKeyword.begin(), queryOfKeyword.end());
            }
        }
    }

    return {Xapian::Query::OP_AND, {Xapian::Query::OP_AND, queries.begin(), queries.end()}, fileOrDir};
}

bool FileSearchWorker::searchWithIndex()
{
    try {
        Xapian::Database db(INDEX_PATH.toStdString());
        Xapian::Enquire enquire(db);
//        qDebug() << "===" << QString::fromStdString(creatQueryForFileSearch().get_description());
        enquire.set_query(creatQueryForFileSearch());
        FileSearchFilter fileSearchFilter(this);

        Xapian::MSet result = enquire.get_mset(0, m_searchController->maxResults(), 0, &fileSearchFilter);

        for (auto it = result.begin(); it != result.end(); ++it) {
            if (m_searchController->beginSearchIdCheck(m_currentSearchId)) {
                QString path = QString::fromStdString(it.get_document().get_data());
                SearchResultProperties properties = m_searchController->getResultProperties(SearchProperty::SearchType::File);
                ResultItem resultItem(m_currentSearchId);
                resultItem.setItemKey(path);
                if(properties.contains(SearchProperty::SearchResultProperty::FilePath)) {
                    resultItem.setValue(SearchProperty::SearchResultProperty::FilePath, path);
                }
                if(properties.contains(SearchProperty::SearchResultProperty::FileIconName)) {
                    resultItem.setValue(SearchProperty::SearchResultProperty::FileIconName, FileUtils::getFileIcon(QUrl::fromLocalFile(path).toString()).name());
                }
                if(properties.contains(SearchProperty::SearchResultProperty::FileName)) {
                    resultItem.setValue(SearchProperty::SearchResultProperty::FileName, path.section("/", -1));
                }
                if(properties.contains(SearchProperty::SearchResultProperty::ModifiedTime)) {
                    resultItem.setValue(SearchProperty::SearchResultProperty::ModifiedTime,
                                        QDateTime::fromString(QString::fromStdString(it.get_document().get_value(2)), "yyyyMMddHHmmsszzz"));
                }
                m_searchController->getDataQueue()->enqueue(resultItem);
                if(++m_resultNum == m_searchController->informNum()) {
                    QMetaObject::invokeMethod(m_FileSearchTask, "reachInformNum");
                    m_resultNum = 0;
                }
                m_searchController->finishSearchIdCheck();

            } else {
                qDebug() << "Search id changed!";
                m_searchController->finishSearchIdCheck();
                return false;
            }
        }

    } catch(const Xapian::Error &e) {
        qWarning() << QString::fromStdString(e.get_description());
        sendErrorMsg("Xapian Error: " + QString::fromStdString(e.get_description()));
        return false;
    }

    return true;
}

void FileSearchWorker::sendErrorMsg(const QString &msg)
{
    QMetaObject::invokeMethod(m_FileSearchTask, "searchError",
                              Q_ARG(size_t, m_currentSearchId),
                              Q_ARG(QString, msg));
}

bool FileSearchWorker::directSearch()
{
    unsigned int maxResults = m_searchController->maxResults();
    QQueue<QString> searchPathQueue;
    for (QString &dir : m_validDirectories) {
        searchPathQueue.enqueue(dir);
    }

    QDir dir;
    QFileInfoList infoList;
    QDir::Filters filters;
    if (m_searchController->isSearchDirOnly()) {
        filters = QDir::Dirs | QDir::NoDotAndDotDot;
    } else {
        filters = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot;
        dir.setSorting(QDir::DirsFirst);
    }
    if(m_searchController->searchHiddenFiles()) {
        filters |= QDir::Hidden;
    }
    dir.setFilter(filters);
    while (!searchPathQueue.empty()) {
        dir.setPath(searchPathQueue.dequeue());
        if (!dir.exists())
            continue;
        infoList = dir.entryInfoList();
        for (const auto &fileInfo : infoList) {
            if (fileInfo.isDir() && !fileInfo.isSymLink()) {
                QString newPath = fileInfo.absoluteFilePath();

                bool inBlackList = std::any_of(m_blackList.begin(), m_blackList.end(), [&] (const QString &dir) {
                    return newPath.startsWith(dir + "/");
                });

                if (inBlackList) {
                    //在黑名单的路径忽略搜索
                    continue;
                }
                if (m_searchController->isRecurse()) {
                    searchPathQueue.enqueue(newPath);
                }
                if (m_searchController->isSearchFileOnly()) {
                    continue;
                }
            }

            bool matched = false;
            if (m_searchController->getKeyword().isEmpty() && m_searchController->getMatchAllIfNoKeyword()) {
                matched = true;
            } else {
                //同时包含几个key为成功匹配
                for (const QString &keyword: m_searchController->getKeyword()) {
                    if (!keyword.isEmpty()) {
                        //TODO 修改匹配方式,对结果进行排序
                        if (fileInfo.fileName().contains(keyword, Qt::CaseInsensitive)) {
                            matched = true;
                            break;
                        }
                    }
                }
            }

            if (matched && !m_labels.empty()) {
                matched = FileSearchFilter::checkFileLabel(fileInfo.absoluteFilePath(), m_labels);
            }

            if (m_searchController->beginSearchIdCheck(m_currentSearchId)) {
                if (matched) {
                    ResultItem ri(m_currentSearchId);
                    ri.setItemKey(fileInfo.absoluteFilePath());
                    SearchResultProperties properties = m_searchController->getResultProperties(SearchProperty::SearchType::File);
                    if(properties.contains(SearchProperty::SearchResultProperty::FilePath)) {
                        ri.setValue(SearchProperty::SearchResultProperty::FilePath, fileInfo.absoluteFilePath());
                    }
                    if(properties.contains(SearchProperty::SearchResultProperty::FileIconName)) {
                        ri.setValue(SearchProperty::SearchResultProperty::FileIconName, FileUtils::getFileIcon(QUrl::fromLocalFile(fileInfo.absoluteFilePath()).toString()).name());
                    }
                    if(properties.contains(SearchProperty::SearchResultProperty::FileName)) {
                        ri.setValue(SearchProperty::SearchResultProperty::FileName, fileInfo.fileName());
                    }
                    if(properties.contains(SearchProperty::SearchResultProperty::ModifiedTime)) {
                        ri.setValue(SearchProperty::SearchResultProperty::ModifiedTime, fileInfo.lastModified());
                    }
                    m_searchController->getDataQueue()->enqueue(ri);
                    if(++m_resultNum == m_searchController->informNum()) {
                        QMetaObject::invokeMethod(m_FileSearchTask, "reachInformNum");
                        m_resultNum = 0;
                    }
                    --maxResults;
                }
                m_searchController->finishSearchIdCheck();
                if (maxResults == 0) {
                    return true;
                }
            } else {
                qDebug() << "Search id changed!";
                m_searchController->finishSearchIdCheck();
                return false;
            }
        }
    }

    return true;
}

FileSearchFilter::FileSearchFilter(FileSearchWorker *parent) : m_parent(parent) {}

bool FileSearchFilter::operator ()(const Xapian::Document &doc) const
{
    if (m_parent) {
        QString path = QString::fromStdString(doc.get_data());
        bool isRecurse = m_parent->m_searchController->isRecurse();
        bool inSearchDir = true;

        if (!m_parent->m_validDirectories.empty()) {
            inSearchDir = std::any_of(m_parent->m_validDirectories.begin(), m_parent->m_validDirectories.end(), [&](QString &dir) {
                //限制搜索在该目录下
                if (dir != "/" && !path.startsWith(dir + "/")) {
                    return false;
                }

                if (!isRecurse) {
                    //去除搜索路径后，是否包含 "/"
                    return !path.midRef((dir.length() + 1), (path.length() - dir.length() - 1)).contains("/");
                }

                return true;
            });
        }

        bool hasLabel = true;
        if (inSearchDir && !m_parent->m_labels.empty()) {
            hasLabel = FileSearchFilter::checkFileLabel(path, m_parent->m_labels);
        }

        return inSearchDir && hasLabel;
    }

    return true;
}

bool FileSearchFilter::checkFileLabel(const QString &path, const QStringList &labels)
{
    bool hasLabel = false;

    GFile *file = g_file_new_for_path(path.toUtf8().constData());
    if (file) {
        GFileInfo *info = g_file_query_info(file, "metadata::*," G_FILE_ATTRIBUTE_ID_FILE, G_FILE_QUERY_INFO_NONE,
                                            nullptr, nullptr);
        if (info) {
            gboolean hasKey = g_file_info_has_attribute(info, "metadata::explor-file-label-ids");
            if (hasKey) {
                char *fileLabels = g_file_info_get_attribute_as_string(info, "metadata::explor-file-label-ids");
                if (fileLabels) {
                    QStringList fileLabelList = QString::fromUtf8(fileLabels).split("\n");
                    g_free(fileLabels);

                    hasLabel = std::any_of(fileLabelList.begin(), fileLabelList.end(), [&](QString &fileLabel) {
                        return labels.contains(fileLabel);
                    });
                }
            }

            g_object_unref(info);
        }

        g_object_unref(file);
    }

    return hasLabel;
}
