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

//lingmo-search
#include "file-content-search-task.h"
#include "index-status-recorder.h"
#include "chinese-segmentation.h"
#include "result-item.h"
#include "common.h"
#include "file-utils.h"
#include "file-indexer-config.h"

//Qt
#include <QIcon>
#include <QDebug>
#include <QDateTime>

using namespace LingmoUISearch;

FileContentSearchTask::FileContentSearchTask(QObject *parent)
{
    this->setParent(parent);
    qRegisterMetaType<size_t>("size_t");
    m_pool = new QThreadPool(this);
    m_pool->setMaxThreadCount(1);
}

FileContentSearchTask::~FileContentSearchTask()
{
    m_pool->clear();
    m_pool->waitForDone();
}

void FileContentSearchTask::setController(const SearchController &searchController)
{
    m_searchController = searchController;
}

PluginInterface::PluginType FileContentSearchTask::pluginType()
{
    return PluginType::SearchTaskPlugin;
}

const QString FileContentSearchTask::name()
{
    return tr("File Content");
}

const QString FileContentSearchTask::description()
{
    return tr("File Content Search");
}

const QIcon FileContentSearchTask::icon()
{
    return QIcon::fromTheme("folder", QIcon(":/unknown.svg"));
}

void FileContentSearchTask::setEnable(bool enable)
{
    m_enable = enable;
}

bool FileContentSearchTask::isEnable()
{
    int state = IndexStatusRecorder::getInstance()->getStatus(CONTENT_INDEX_DATABASE_STATE_KEY).toInt();
    return m_enable && state == IndexStatusRecorder::State::Ready || state == IndexStatusRecorder::State::Updating || state == IndexStatusRecorder::State::RealTimeUpdating;
}

QString FileContentSearchTask::getCustomSearchType()
{
    return tr("File Content");
}

SearchProperty::SearchType FileContentSearchTask::getSearchType()
{
    return SearchProperty::SearchType::FileContent;
}

void FileContentSearchTask::startSearch()
{
    FileContentSearchWorker *worker = new FileContentSearchWorker(this, &m_searchController);
    m_pool->start(worker);
}

void FileContentSearchTask::stop()
{

}

bool FileContentSearchTask::isSearching()
{
    return m_pool->activeThreadCount() > 0;
}

FileContentSearchWorker::FileContentSearchWorker(FileContentSearchTask *fileContentSearchTask, SearchController *searchController)
{
    m_fileContentSearchTask = fileContentSearchTask;
    m_searchController = searchController;
    m_currentSearchId = m_searchController->getCurrentSearchId();
}

void FileContentSearchWorker::run()
{
    QStringList searchDirs = m_searchController->getSearchDir();
    searchDirs.removeDuplicates();

    for (QString &dir : searchDirs) {
        if (dir.endsWith("/") && dir != "/") {
            dir = dir.mid(0, dir.length() - 1);
        }

        m_validDirectories.append(dir);
    }

    bool finished = true;
    if (m_fileContentSearchTask->isEnable()) {
        qDebug() << "content index enable";
        finished = execSearch();

    } else {
        qWarning() << "content index incomplete";
        sendErrorMsg(QObject::tr("Content index incomplete."));

        finished = false;
    }

    if (finished) QMetaObject::invokeMethod(m_fileContentSearchTask, "searchFinished", Q_ARG(size_t, m_currentSearchId));
}

void FileContentSearchWorker::sendErrorMsg(const QString &msg)
{
    QMetaObject::invokeMethod(m_fileContentSearchTask, "searchError",
                              Q_ARG(size_t, m_currentSearchId),
                              Q_ARG(QString, msg));
}

bool FileContentSearchWorker::execSearch()
{
    try {
        Xapian::Database db(CONTENT_INDEX_PATH.toStdString());
        if(FileIndexerConfig::getInstance()->isOCREnable()) {
            db.add_database(Xapian::Database(OCR_CONTENT_INDEX_PATH.toStdString()));
        }

        Xapian::Enquire enquire(db);

        enquire.set_query(createQuery());

        FileContentSearchFilter filter(this);

        Xapian::MSet result = enquire.get_mset(0, m_searchController->maxResults(), nullptr, &filter);

        for (auto it = result.begin(); it != result.end(); ++it) {
            if (m_searchController->beginSearchIdCheck(m_currentSearchId)) {
                QString path = QString::fromStdString(it.get_document().get_value(CONTENT_DATABASE_PATH_SLOT));

                SearchResultProperties properties = m_searchController->getResultProperties(SearchProperty::SearchType::FileContent);
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
                                        QDateTime::fromString(QString::fromStdString(it.get_document().get_value(3)), "yyyyMMddHHmmsszzz"));
                }
                m_searchController->getDataQueue()->enqueue(resultItem);
                if(++m_resultNum == m_searchController->informNum()) {
                    QMetaObject::invokeMethod(m_fileContentSearchTask, "reachInformNum");
                    m_resultNum = 0;
                }
                m_searchController->finishSearchIdCheck();

            } else {
                qDebug() << "Search id changed!";
                m_searchController->finishSearchIdCheck();
                return false;
            }
        }

        return true;

    } catch (const Xapian::Error &e) {
        qWarning() << QString::fromStdString(e.get_description());
        sendErrorMsg("Xapian Error: " + QString::fromStdString(e.get_description()));
        return false;
    }
}

inline Xapian::Query FileContentSearchWorker::createQuery()
{
    std::vector<Xapian::Query> v;

    for (auto &keyword : m_searchController->getKeyword()) {
        std::vector<KeyWord> sKeyWord = ChineseSegmentation::getInstance()->callSegment(keyword);

        for(const auto & c : sKeyWord) {
            v.emplace_back(c.word);
        }
    }
    Xapian::Query query;
    if(FileIndexerConfig::getInstance()->isFuzzySearchEnable()) {
        query = Xapian::Query(Xapian::Query::OP_OR, v.begin(), v.end());
    } else {
        query = Xapian::Query(Xapian::Query::OP_AND, v.begin(), v.end());
    }

    return query;
}


FileContentSearchFilter::FileContentSearchFilter(FileContentSearchWorker *worker) : m_worker(worker)
{

}

bool FileContentSearchFilter::operator()(const Xapian::Document &doc) const
{
    //在此处对搜索结果进行过滤
    QString path = QString::fromStdString(doc.get_value(CONTENT_DATABASE_PATH_SLOT));
    if(!QFileInfo::exists(path)) {
        return false;
    }

    //如果不指定搜索目录，那么搜索整个数据库
    if (m_worker && !m_worker->m_validDirectories.empty()) {
        for(const QString &dir : m_worker->m_validDirectories) {
            return FileUtils::isOrUnder(path, dir);
        }
    }

    return true;
}
