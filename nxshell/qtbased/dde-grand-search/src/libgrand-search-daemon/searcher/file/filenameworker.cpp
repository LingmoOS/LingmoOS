// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenameworker_p.h"
#include "global/builtinsearch.h"
#include "global/commontools.h"
#include "utils/specialtools.h"
#include "global/searchhelper.h"
#include "configuration/configer.h"

#include "interfaces/anything_interface.h"

#include <QStandardPaths>

using namespace GrandSearch;

#define MAX_SEARCH_NUM 100
#define EMIT_INTERVAL 50

FileNameWorkerPrivate::FileNameWorkerPrivate(FileNameWorker *qq)
    : q_ptr(qq)
{
    // 搜索目录为user
    QStringList homePaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (!homePaths.isEmpty())
        m_searchPath = homePaths.first();

    initAnything();
    initConfig();
}

void FileNameWorkerPrivate::initConfig()
{
    // 获取支持的搜索类目
    auto config = ConfigerIns->group(GRANDSEARCH_CLASS_FILE_DEEPIN);
    if (config->value(GRANDSEARCH_GROUP_FOLDER, false))
        m_resultCountHash.insert(FileSearchUtils::Folder, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE, false))
        m_resultCountHash.insert(FileSearchUtils::File, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_VIDEO, false))
        m_resultCountHash.insert(FileSearchUtils::Video, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_AUDIO, false))
        m_resultCountHash.insert(FileSearchUtils::Audio, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_PICTURE, false))
        m_resultCountHash.insert(FileSearchUtils::Picture, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_DOCUMNET, false))
        m_resultCountHash.insert(FileSearchUtils::Document, 0);
}

void FileNameWorkerPrivate::initAnything()
{
    Q_Q(FileNameWorker);

    m_anythingInterface = new ComDeepinAnythingInterface("com.deepin.anything",
                                                         "/com/deepin/anything",
                                                         QDBusConnection::systemBus(),
                                                         q);
    m_anythingInterface->setTimeout(1000);

    // 自动索引内置磁盘
    if (!m_anythingInterface->autoIndexInternal())
        m_anythingInterface->setAutoIndexInternal(true);
}

QFileInfoList FileNameWorkerPrivate::traverseDirAndFile(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        return {};

    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    auto result = dir.entryInfoList();
    // 排序
    qSort(result.begin(), result.end(), [](const QFileInfo &info1, const QFileInfo &info2) {
        static QStringList sortList{"Desktop", "Music", "Downloads", "Documents", "Pictures", "Videos"};
        int index1 = sortList.indexOf(info1.fileName());
        int index2 = sortList.indexOf(info2.fileName());

        if (index1 == -1)
            return false;
        else if (index2 == -1)
            return true;

        return index1 < index2;
    });

    return result;
}

bool FileNameWorkerPrivate::appendSearchResult(const QString &fileName)
{
    Q_Q(FileNameWorker);

    if (m_tmpSearchResults.contains(fileName))
        return false;

    auto group = FileSearchUtils::getGroupByName(fileName);
    Q_ASSERT(group >= FileSearchUtils::GroupBegin && group< FileSearchUtils::GroupCount);

    // 根据搜索类目配置判断是否需要进行添加
    if (!m_resultCountHash.contains(group)) {
        if (group == FileSearchUtils::Folder)
            return false;

        if (m_resultCountHash.contains(FileSearchUtils::File)) {
            group = FileSearchUtils::File;
        } else {
            return false;
        }
    }

    if (!FileSearchUtils::fileShouldVisible(fileName, group, m_searchInfo))
        return false;

    if (m_resultCountHash[group] >= MAX_SEARCH_NUM || FileSearchUtils::filterByBlacklist(fileName))
        return false;

    m_tmpSearchResults << fileName;
    const auto &item = FileSearchUtils::packItem(fileName, q->name());
    QMutexLocker lk(&m_mutex);
    m_items[group].append(item);
    m_resultCountHash[group]++;

    // 非文件类目搜索，不需要向文件类目中添加搜索结果
    if (m_searchInfo.isCombinationSearch && !m_searchInfo.groupList.contains(FileSearchUtils::File))
        return true;

    // 文档、音频、视频、图片需添加到文件组中
    if (group != FileSearchUtils::File && m_resultCountHash.contains(FileSearchUtils::File)) {
        if (group != FileSearchUtils::Folder && m_resultCountHash[FileSearchUtils::File] < MAX_SEARCH_NUM) {
            m_items[FileSearchUtils::File].append(item);
            m_resultCountHash[FileSearchUtils::File]++;
        }
    }

    return true;
}

bool FileNameWorkerPrivate::searchUserPath()
{
    QFileInfoList fileInfoList = traverseDirAndFile(m_searchPath);
    // 先对user目录下进行搜索
    for (const auto &info : fileInfoList) {
        //中断
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return false;

        if (info.isDir())
            m_searchDirList << info.absoluteFilePath();

        QRegExp reg(m_searchInfo.keyword, Qt::CaseInsensitive);
        if (info.fileName().contains(reg)) {
            auto absoluteFilePath = info.absoluteFilePath();

            // 过滤文管设置的隐藏文件
            if (SpecialTools::isHiddenFile(absoluteFilePath, m_hiddenFilters, QDir::homePath()))
                continue;

            // 去除掉添加的data前缀
            if (m_hasTransformed && absoluteFilePath.startsWith(m_searchPath))
                absoluteFilePath.replace(m_searchPath, m_originalSearchPath);

            appendSearchResult(absoluteFilePath);

            //推送
            tryNotify();

            if (isResultLimit())
                break;
        }
    }

    int leave = itemCount();
    qInfo() << "user path search found items:" << m_resultCountHash
            << "total spend:" << m_time.elapsed()
            << "current items" << leave;

    return true;
}

bool FileNameWorkerPrivate::searchByAnything()
{
    // 搜索
    quint32 searchStartOffset = 0;
    quint32 searchEndOffset = 0;
    // 过滤系统隐藏文件
    QRegExp hiddenFileFilter("^(?!.*/\\..*).+$");
    while (!isResultLimit() && !m_searchDirList.isEmpty()) {
        //中断
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return false;

        QDBusPendingReply<QStringList, uint, uint> result;
        if (m_supportParallelSearch) {
            QStringList rules;
            rules << "0x02100"  // 搜索做大数量，100
                  << "0x40."    // 过滤系统隐藏文件
                  << "0x011"    // 支持正则表达式
                  << "0x031"    // 忽略大小写
                  << "0x061";   // 拼音搜索
            result = m_anythingInterface->parallelsearch(m_searchDirList.first(), searchStartOffset,
                                                         searchEndOffset, m_searchInfo.keyword, rules);
        } else {
            result = m_anythingInterface->search(100, 100, searchStartOffset,
                                                 searchEndOffset, m_searchDirList.first(), m_searchInfo.keyword,
                                                 true);
        }

        // fix bug 93806
        // 直接判断errorType为NoError，需要先取值再判断
        QStringList searchResults = result.argumentAt<0>();
        if (result.error().type() != QDBusError::NoError) {
            qWarning() << "deepin-anything search failed:"
                       << QDBusError::errorString(result.error().type())
                       << result.error().message();
            searchStartOffset = searchEndOffset = 0;
            m_searchDirList.removeAt(0);
            continue;
        }

        if (!m_supportParallelSearch)
            searchResults = searchResults.filter(hiddenFileFilter);
        searchStartOffset = result.argumentAt<1>();
        searchEndOffset = result.argumentAt<2>();

        // 当前目录已经搜索到了结尾
        if (searchStartOffset >= searchEndOffset) {
            searchStartOffset = searchEndOffset = 0;
            m_searchDirList.removeAt(0);
        }

        for (auto &path : searchResults) {
            //中断
            if (m_status.loadAcquire() != ProxyWorker::Runing)
                return false;

            // 去除掉添加的data前缀
            if (m_hasTransformed && path.startsWith(m_searchPath))
                path.replace(m_searchPath, m_originalSearchPath);

            // 过滤文管设置的隐藏文件
            if (SpecialTools::isHiddenFile(path, m_hiddenFilters, QDir::homePath()))
                continue;

            appendSearchResult(path);

            //推送
            tryNotify();

            if (isResultLimit())
                break;
        }
    }

    int leave = itemCount();
    qInfo() << "anything search completed, found items:" << m_resultCountHash
            << "total spend:" << m_time.elapsed()
            << "current items" << leave;

    return true;
}

void FileNameWorkerPrivate::tryNotify()
{
    Q_Q(FileNameWorker);
    int cur = m_time.elapsed();
    if (q->hasItem() && (cur - m_lastEmit) > EMIT_INTERVAL) {
        m_lastEmit = cur;
        qDebug() << "unearthed, current spend:" << cur;
        emit q->unearthed(q);
    }
}

int FileNameWorkerPrivate::itemCount() const
{
    QMutexLocker lk(&m_mutex);

    int count = 0;
    for (int i = FileSearchUtils::GroupBegin; i < FileSearchUtils::GroupCount; ++i)
        count += m_items[i].size();

    return count;
}

bool FileNameWorkerPrivate::isResultLimit()
{
    const auto &iter = std::find_if(m_resultCountHash.begin(), m_resultCountHash.end(), [](const int &num){
        return num <= MAX_SEARCH_NUM;
    });

    return iter == m_resultCountHash.end();
}

FileNameWorker::FileNameWorker(const QString &name, bool supportParallelSearch, QObject *parent)
    : ProxyWorker(name, parent),
      d_ptr(new FileNameWorkerPrivate(this))
{
    d_ptr->m_supportParallelSearch = supportParallelSearch;
}

FileNameWorker::~FileNameWorker()
{
    delete d_ptr;
    d_ptr = nullptr;
}

void FileNameWorker::setContext(const QString &context)
{
    Q_D(FileNameWorker);

    if (context.isEmpty())
        qWarning() << "search key is empty.";
    d->m_searchInfo = FileSearchUtils::parseContent(context);
}

bool FileNameWorker::isAsync() const
{
    return false;
}

bool FileNameWorker::working(void *context)
{
    Q_D(FileNameWorker);
    Q_UNUSED(context)

    //准备状态切运行中，否则直接返回
    if (!d->m_status.testAndSetRelease(Ready, Runing))
        return false;

    if (!d->m_anythingInterface->isValid() || d->m_searchInfo.keyword.isEmpty() || d->m_searchPath.isEmpty()) {
        d->m_status.storeRelease(Completed);
        return false;
    }

    d->m_time.start();

    //检查home路径
    bool useAnything = true;
    if (!d->m_anythingInterface->hasLFT(d->m_searchPath)) {
        // 有可能 anything 不支持/home目录，但是支持/data/home
        const QString &tmpPath = CommonTools::bindPathTransform(d->m_searchPath, true);
        if (!d->m_anythingInterface->hasLFT(tmpPath)) {
            qWarning() << "Do not support quick search for " << tmpPath;
            useAnything = false;
        } else {
            d->m_originalSearchPath = d->m_searchPath;
            d->m_searchPath = tmpPath;
            d->m_hasTransformed = true;
        }
    }

    if (!d->m_supportParallelSearch) {
        // 搜索user目录下文件
        if (!d->searchUserPath())
            return false; //中断
    } else {
        d->m_searchDirList << d->m_searchPath;
    }

    // 使用anything搜索
    if (useAnything) {
        if (!d->searchByAnything())
            return false; //中断
    }

    //检查是否还有数据
    if (d->m_status.testAndSetRelease(Runing, Completed)) {
        //发送数据
        if (hasItem())
            emit unearthed(this);
    }
    return true;
}

void FileNameWorker::terminate()
{
    Q_D(FileNameWorker);

    d->m_status.storeRelease(Terminated);
}

ProxyWorker::Status FileNameWorker::status()
{
    Q_D(FileNameWorker);

    return static_cast<ProxyWorker::Status>(d->m_status.loadAcquire());
}

bool FileNameWorker::hasItem() const
{
    Q_D(const FileNameWorker);

    QMutexLocker lk(&d->m_mutex);
    for (int i = FileSearchUtils::GroupBegin; i < FileSearchUtils::GroupCount; ++i)
        if (!d->m_items[i].isEmpty())
            return true;

    return false;
}

MatchedItemMap FileNameWorker::takeAll()
{
    Q_D(FileNameWorker);

    //添加分组
    MatchedItemMap ret;

    QMutexLocker lk(&d->m_mutex);
    for (int i = FileSearchUtils::GroupBegin; i < FileSearchUtils::GroupCount; ++i) {
        if (!d->m_items[i].isEmpty()) {
            MatchedItems items = std::move(d->m_items[i]);
            Q_ASSERT(d->m_items[i].isEmpty());
            ret.insert(FileSearchUtils::groupKey(static_cast<FileSearchUtils::Group>(i)), items);
        }
    }
    lk.unlock();

    return ret;
}


