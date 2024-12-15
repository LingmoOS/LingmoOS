// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesearchutils.h"
#include "global/builtinsearch.h"
#include "utils/specialtools.h"
#include "global/searchhelper.h"
#include "global/searchconfigdefine.h"
#include "configuration/configer.h"

using namespace GrandSearch;

MatchedItem FileSearchUtils::packItem(const QString &fileName, const QString &searcher)
{
    QFileInfo fileInfo(fileName);
    QMimeType mimeType = SpecialTools::getMimeType(fileInfo);
    GrandSearch::MatchedItem item;
    item.item = fileName;
    item.name = fileInfo.fileName();
    item.type = mimeType.name();
    item.icon = mimeType.iconName();
    item.searcher = searcher;
    item.extra = QVariant::fromValue(tailerData(fileInfo));

    return item;
}

QString FileSearchUtils::groupKey(FileSearchUtils::Group group)
{
    switch (group) {
    case Folder:
        return GRANDSEARCH_GROUP_FOLDER;
    case Picture:
        return GRANDSEARCH_GROUP_FILE_PICTURE;
    case Audio:
        return GRANDSEARCH_GROUP_FILE_AUDIO;
    case Video:
        return GRANDSEARCH_GROUP_FILE_VIDEO;
    case Document:
        return GRANDSEARCH_GROUP_FILE_DOCUMNET;
    default:
        break;
    }
    return GRANDSEARCH_GROUP_FILE;
}

FileSearchUtils::Group FileSearchUtils::getGroupByName(const QString &fileName)
{
    Group group = Unknown;
    QFileInfo fileInfo(fileName);

    if (fileInfo.isDir()) {
        group = Folder;
    } else {
        group = getGroupBySuffix(fileInfo.suffix());
    }

    return group;
}

FileSearchUtils::Group FileSearchUtils::getGroupBySuffix(const QString &suffix)
{
    Group group = Unknown;
    if (suffix.isEmpty())
        return group;

    if (searchHelper->getSuffixByGroupName(DOCUMENT_GROUP).contains(suffix, Qt::CaseInsensitive)) {
        group = Document;
    } else if (searchHelper->getSuffixByGroupName(PICTURE_GROUP).contains(suffix, Qt::CaseInsensitive)) {
        group = Picture;
    } else if (searchHelper->getSuffixByGroupName(VIDEO_GROUP).contains(suffix, Qt::CaseInsensitive)) {
        group = Video;
    } else if (searchHelper->getSuffixByGroupName(AUDIO_GROUP).contains(suffix, Qt::CaseInsensitive)) {
        group = Audio;
    } else if (searchHelper->getSuffixByGroupName(FILE_GROUP).contains(suffix, Qt::CaseInsensitive)) {
        group = File;
    }

    return group;
}

FileSearchUtils::Group FileSearchUtils::getGroupByGroupName(const QString &groupName)
{
    Group group = Unknown;
    if (groupName.compare(FOLDER_GROUP, Qt::CaseInsensitive) == 0) {
        group = Folder;
    } else if (groupName.compare(FILE_GROUP, Qt::CaseInsensitive) == 0) {
        group = File;
    } else if (groupName.compare(VIDEO_GROUP, Qt::CaseInsensitive) == 0) {
        group = Video;
    } else if (groupName.compare(AUDIO_GROUP, Qt::CaseInsensitive) == 0) {
        group = Audio;
    } else if (groupName.compare(PICTURE_GROUP, Qt::CaseInsensitive) == 0) {
        group = Picture;
    } else if (groupName.compare(DOCUMENT_GROUP, Qt::CaseInsensitive) == 0) {
        group = Document;
    }

    return group;
}

FileSearchUtils::SearchInfo FileSearchUtils::parseContent(const QString &content)
{
    SearchInfo info;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(content.toLocal8Bit(), &error);
    if (error.error != QJsonParseError::NoError || doc.isEmpty()) {
        info.keyword = searchHelper->tropeInputSymbol(content);
        return info;
    }

    QStringList keywordList;
    QJsonObject obj = doc.object();
    // 类目
    QJsonArray groupArr = obj[JSON_GROUP_ATTRIBUTE].toArray();
    for (int i = 0; i < groupArr.count(); ++i) {
        const QString &group = groupArr[i].toString();
        if (group.isEmpty())
            continue;

        auto groupType = getGroupByGroupName(group);
        if (groupType != Unknown)
            info.groupList << groupType;
    }

    // 后缀
    QJsonArray suffixArr = obj[JSON_SUFFIX_ATTRIBUTE].toArray();
    for (int i = 0; i < suffixArr.count(); ++i) {
        const QString &suffix = suffixArr[i].toString();
        if (suffix.isEmpty())
            continue;
        info.suffixList << suffix;
    }

    // 类目/后缀表不为空，说明当前搜索方式为组合搜索
    if (!info.groupList.isEmpty() || !info.suffixList.isEmpty())
        info.isCombinationSearch = true;

    // 搜索关键字
    QJsonArray keywordArr = obj[JSON_KEYWORD_ATTRIBUTE].toArray();
    for (int i = 0; i < keywordArr.count(); ++i) {
        const QString &key = keywordArr[i].toString();
        if (key.isEmpty())
            continue;
        keywordList.append(searchHelper->tropeInputSymbol(key));
    }

    info.keyword = QString(R"((%1).*)").arg(keywordList.join('|'));
    return info;
}

bool FileSearchUtils::fileShouldVisible(const QString &fileName, Group &group, const FileSearchUtils::SearchInfo &info)
{
    // 对组合搜索到的结果进行过滤
    if (info.isCombinationSearch) {
        if (!info.groupList.contains(group)) {
            QFileInfo fileInfo(fileName);
            if (fileInfo.isDir())
                return false;

            const auto &suffix = fileInfo.suffix();
            if (suffix.isEmpty() || !info.suffixList.contains(suffix, Qt::CaseInsensitive)) {
                if (info.groupList.contains(File)) {
                    group = File;
                    return true;
                }

                return false;
            }
        }
    }

    return true;
}

bool FileSearchUtils::filterByBlacklist(const QString &fileName)
{
    // 过滤黑名单中的结果
    // add "/" to the end of fileName to distinguish partially identical path
    QString filePath = fileName + "/";
    auto config = Configer::instance()->group(GRANDSEARCH_BLACKLIST_GROUP);
    auto blacklist = config->value(GRANDSEARCH_BLACKLIST_PATH, QStringList());
    if (!blacklist.isEmpty()) {
        for (const auto &path : blacklist) {
            if (filePath.startsWith(path))
                return true;
        }
    }

    return false;
}

QVariantHash FileSearchUtils::tailerData(const QFileInfo &info)
{
    QVariantHash hash;
    QStringList datas;
    auto config = Configer::instance()->group(GRANDSEARCH_TAILER_GROUP);
    if (config->value(GRANDSEARCH_TAILER_PARENTDIR, false))
        datas.append(info.absolutePath());

    if (config->value(GRANDSEARCH_TAILER_TIMEMODEFIED, false)) {
        auto timeModified = info.lastModified().toString("yyyy-MM-dd hh:mm ") + QObject::tr("modified");
        datas.append(timeModified);
    }

    if (!datas.isEmpty())
        hash.insert(GRANDSEARCH_PROPERTY_ITEM_TAILER, datas);

    return hash;
}
