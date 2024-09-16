// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fileresultshandler.h"

#include "configuration/configer.h"

using namespace GrandSearch;

#define MAX_SEARCH_NUM 300

FileResultsHandler::FileResultsHandler()
{
    initConfig();
}

#if 0
bool FileResultsHandler::appendTo(const QString &file, MatchedItemMap &container)
{
    // read locker
    FileSearchUtils::Group group;
    {
        QReadLocker lk(&m_lock);
        if (m_tmpSearchResults.contains(file))
            return false;

        group = FileSearchUtils::getGroupByName(file);
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

        if (m_resultCountHash[group] >= MAX_SEARCH_NUM || FileSearchUtils::filterByBlacklist(file))
            return false;
    }

    auto item = FileSearchUtils::packItem(file, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC);

    QWriteLocker lk(&m_lock);
    m_tmpSearchResults << file;
    m_resultCountHash[group]++;
    container[FileSearchUtils::groupKey(group)].append(item);

    // 文档、音频、视频、图片还需添加到文件组中
    if (group != FileSearchUtils::File && group != FileSearchUtils::Folder
            && m_resultCountHash.contains(FileSearchUtils::File)
            && m_resultCountHash[FileSearchUtils::File] < MAX_SEARCH_NUM) {
            m_resultCountHash[FileSearchUtils::File]++;
    }

    return true;
}

bool FileResultsHandler::isResultLimit() const
{
    QReadLocker lk(&m_lock);
    const auto &iter = std::find_if(m_resultCountHash.begin(), m_resultCountHash.end(), [](const int &num){
        return num <= MAX_SEARCH_NUM;
    });

    return iter == m_resultCountHash.end();
}
#else
bool FileResultsHandler::appendTo(const QString &file, MatchedItemMap &container)
{
    // read locker
    {
        QReadLocker lk(&m_lock);
        if (m_tmpSearchResults.contains(file))
            return false;

        if (m_tmpSearchResults.size() >= MAX_SEARCH_NUM || FileSearchUtils::filterByBlacklist(file))
            return false;
    }

    auto item = FileSearchUtils::packItem(file, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC);

    QWriteLocker lk(&m_lock);
    m_tmpSearchResults << file;
    container[GRANDSEARCH_GROUP_FILE_INFERENCE].append(item);
    return true;
}

bool FileResultsHandler::isResultLimit() const
{
    QReadLocker lk(&m_lock);
    return m_tmpSearchResults.size() >= MAX_SEARCH_NUM;
}

#endif

void FileResultsHandler::appendExtra(const QString &file, const QString &key, const QVariant &value)
{
    QReadLocker lk(&m_lock);
    auto it = m_resultExtra.find(file);
    if (it != m_resultExtra.end()) {
        it.value().insert(key, value);
    } else {
        QVariantHash tmp = {{key, value}};
        m_resultExtra.insert(file, tmp);
    }
}

void FileResultsHandler::initConfig()
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
