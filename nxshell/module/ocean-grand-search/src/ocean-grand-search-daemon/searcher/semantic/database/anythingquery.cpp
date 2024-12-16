// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "anythingquery_p.h"
#include "utils/specialtools.h"
#include "global/searchhelper.h"
#include "searcher/file/filesearchutils.h"
#include "global/builtinsearch.h"
#include "searcher/semantic/fileresultshandler.h"
#include "global/commontools.h"

using namespace GrandSearch;

AnythingQueryPrivate::AnythingQueryPrivate(AnythingQuery *qq) : q(qq)
{

}

AnythingQueryPrivate::~AnythingQueryPrivate()
{
    delete m_anythingInterface;
    m_anythingInterface = nullptr;
}

void AnythingQueryPrivate::initAnything()
{
    QStringList homePaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (!homePaths.isEmpty())
        m_searchPath = homePaths.first();

    m_anythingInterface = new ComLingmoAnythingInterface("com.lingmo.anything",
                                                         "/com/lingmo/anything",
                                                         QDBusConnection::systemBus());
    m_anythingInterface->setTimeout(1000);

    // 自动索引内置磁盘
    if (!m_anythingInterface->autoIndexInternal())
        m_anythingInterface->setAutoIndexInternal(true);
    return ;
}

bool AnythingQueryPrivate::searchUserPath(PushItemCallBack callBack, void *pdata)
{
    QFileInfoList fileInfoList = traverseDirAndFile(m_searchPath);

    QHash<QString, QSet<QString>> filter;
    MatchedItemMap items;

    // 先对user目录下进行搜索
    for (const SemanticEntity &entity : m_entity) {
        QRegExp reg(getRegExp(entity), Qt::CaseInsensitive);
        for (const auto &info : fileInfoList) {
            // 检查是否需要推送数据与中断
            if (timeToPush()) {
                auto it = std::move(items);
                bool ret = callBack(it, pdata);
                // 有数据放入，更新时间
                if (!it.isEmpty())
                    m_lastPush = m_time.elapsed();

                // 中断
                if (!ret)
                    return false;
            }

            if (info.isDir())
                m_searchDirList << info.absoluteFilePath();

            // 检查文件名
            if (!entity.partPath.isEmpty()) {
                bool isMatch = info.canonicalFilePath().contains(entity.partPath);
                //qDebug() << QString("(%1) vs (%2) isMatch(%3) vs require(%4)").arg(info.canonicalFilePath())
                //            .arg(m_entity.partPath).arg(isMatch).arg(m_entity.isTrue);
                if ((!entity.isContainPath && isMatch) || (entity.isContainPath && !isMatch)) {
                    continue;
                }
            } else {
                if (!info.fileName().contains(reg))
                    continue;
            }

            // 检查时间
            if (!SemanticHelper::isMatchTime(info.lastModified().toSecsSinceEpoch(), entity.times)
                    && !SemanticHelper::isMatchTime(info.birthTime().toSecsSinceEpoch(), entity.times))
                continue;

            // 检查文件大小
            if (!entity.fileCompType.isEmpty()) {
                if (entity.fileCompType == ">") {
                    if (!(info.size() > entity.fileSize)) {
                        continue;
                    }
                } else if (entity.fileCompType == "<") {
                    if (!(info.size() < entity.fileSize)) {
                        continue;
                    }
                } else if (entity.fileCompType == "!=") {
                    if (!(info.size() != entity.fileSize)) {
                        continue;
                    }
                } else {
                    if (!(info.size() == entity.fileSize)) {
                        continue;
                    }
                }
            }

            auto absoluteFilePath = info.absoluteFilePath();

            // 过滤文管设置的隐藏文件
            if (SpecialTools::isHiddenFile(absoluteFilePath, filter, QDir::homePath()))
                continue;

            // 去除掉添加的data前缀
            if (m_hasTransformed && absoluteFilePath.startsWith(m_searchPath))
                absoluteFilePath.replace(m_searchPath, m_originalSearchPath);

            m_count++;
            if (m_handler) {
                m_handler->appendTo(absoluteFilePath, items);
                m_handler->setItemWeight(absoluteFilePath, m_handler->itemWeight(absoluteFilePath) + calcItemWeight(info.fileName()));
                if (m_handler->isResultLimit() || m_count >= 100)
                    break;
            } else {
                auto item = FileSearchUtils::packItem(absoluteFilePath, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC);
                items[GRANDSEARCH_GROUP_FILE_INFERENCE].append(item);
            }
        }
    }

    return callBack(items, pdata);
}

bool AnythingQueryPrivate::searchByAnything(PushItemCallBack callBack, void *pdata)
{
    QStringList dirs = m_searchDirList;
    MatchedItemMap items;

    for (const SemanticEntity &entity : m_entity) {
        QString regStr = getRegExp(entity);
        qDebug() << "anything search reg" << regStr;

        // 搜索
        quint32 searchStartOffset = 0;
        quint32 searchEndOffset = 0;


        while (!dirs.isEmpty()) {
            if (m_handler && m_handler->isResultLimit())
                break;

            QDBusPendingReply<QStringList, uint, uint> result;
            {
                QStringList rules;
                rules << "0x02100"  // 搜索最大数量，100
                      << "0x40."    // 过滤系统隐藏文件
                      << "0x011"    // 支持正则表达式
                      << "0x031";    // 忽略大小写
                result = m_anythingInterface->parallelsearch(dirs.first(), searchStartOffset,
                                                             searchEndOffset, regStr, rules);
            }

            // 直接判断errorType为NoError，需要先取值再判断
            QStringList searchResults = result.argumentAt<0>();
            if (result.error().type() != QDBusError::NoError) {
                qWarning() << "lingmo-anything search failed:"
                           << QDBusError::errorString(result.error().type())
                           << result.error().message();
                searchStartOffset = searchEndOffset = 0;
                dirs.removeAt(0);
                continue;
            }

            searchStartOffset = result.argumentAt<1>();
            searchEndOffset = result.argumentAt<2>();

            // 当前目录已经搜索到了结尾
            if (searchStartOffset >= searchEndOffset) {
                searchStartOffset = searchEndOffset = 0;
                dirs.removeAt(0);
            }

            for (auto &path : searchResults) {
                // 检查是否需要推送数据与中断
                if (timeToPush()) {
                    auto it = std::move(items);
                    bool ret = callBack(it, pdata);
                    // 有数据放入，更新时间
                    if (!it.isEmpty())
                        m_lastPush = m_time.elapsed();

                    // 中断
                    if (!ret)
                        return false;
                }

                // 去除掉添加的data前缀
                if (m_hasTransformed && path.startsWith(m_searchPath))
                    path.replace(m_searchPath, m_originalSearchPath);

                if (!entity.partPath.isEmpty()) {
                    bool isMatch = path.contains(entity.partPath);
                    //qDebug() << QString("(%1) vs (%2) isMatch(%3) vs require(%4)").arg(path)
                    //            .arg(m_entity.partPath).arg(isMatch).arg(m_entity.isTrue);
                    if ((!entity.isContainPath && isMatch) || (entity.isContainPath && !isMatch)) {
                        continue;
                    }
                }

                // 检查时间（创建时间和修改时间的并集）
                QFileInfo info(path);
                if (!SemanticHelper::isMatchTime(info.lastModified().toSecsSinceEpoch(), entity.times)
                        && !SemanticHelper::isMatchTime(info.birthTime().toSecsSinceEpoch(), entity.times))
                    continue;

                // 检查文件大小
                if (!entity.fileCompType.isEmpty()) {
                    if (entity.fileCompType == ">") {
                        if (!(info.size() > entity.fileSize)) {
                            continue;
                        }
                    } else if (entity.fileCompType == "<") {
                        if (!(info.size() < entity.fileSize)) {
                            continue;
                        }
                    } else if (entity.fileCompType == "!=") {
                        if (!(info.size() != entity.fileSize)) {
                            continue;
                        }
                    } else {
                        if (!(info.size() == entity.fileSize)) {
                            continue;
                        }
                    }
                }

                // 过滤文管设置的隐藏文件
                QHash<QString, QSet<QString>> hioceannFilters;
                if (SpecialTools::isHiddenFile(path, hioceannFilters, QDir::homePath()))
                    continue;
                m_count++;
                if (m_handler) {
                    m_handler->appendTo(path, items);
                    m_handler->setItemWeight(path, m_handler->itemWeight(path) + calcItemWeight(info.fileName()));
                    if (m_handler->isResultLimit() || m_count >= 100)
                        break;
                } else {
                    auto item = FileSearchUtils::packItem(path, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC);
                    items[GRANDSEARCH_GROUP_FILE_INFERENCE].append(item);
                }
            }
        }
    }

    return callBack(items, pdata);
}

QFileInfoList AnythingQueryPrivate::traverseDirAndFile(const QString &path)
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

QString AnythingQueryPrivate::getRegExp(const SemanticEntity &entity) const
{
    QString regStr;
    // 文件名
    if (!entity.keys.isEmpty())
        regStr = QString(R"((%0).*)").arg(entity.keys.join('|'));
    else
        regStr = R"(.*)";

    // 后缀名
    QStringList suffixs = SemanticHelper::typeTosuffix(entity.types);
    if (!suffixs.isEmpty()) {
        regStr += QString(R"(\.(%0)$)").arg(suffixs.join('|'));
    } else if (!entity.suffix.isEmpty()) {
        if (entity.isContainType) {
            regStr += QString(R"(\.(%0)$)").arg(entity.suffix);
        } else {
            regStr += QString(R"(\.[^(%0)]$)").arg(entity.suffix);
        }
    }

    return regStr;
}

bool AnythingQueryPrivate::timeToPush() const
{
    return (m_time.elapsed() - m_lastPush) > 100;
}

double AnythingQueryPrivate::calcItemWeight(const QString &name)
{
    double w = 0;

    QSet<QString> mergedKeys;
    for (const SemanticEntity &e: m_entity) {
        auto keys = e.keys;
        for (const QString &key : keys)
            mergedKeys.insert(key);
    }

    for ( const QString &key : mergedKeys) {
        if (name.contains(key, Qt::CaseInsensitive))
            w += 15;
    }

    return w;
}

AnythingQuery::AnythingQuery(QObject *parent)
    : QObject(parent)
    , d(new AnythingQueryPrivate(this))
{
    d->initAnything();
}

AnythingQuery::~AnythingQuery()
{
    delete d;
    d = nullptr;
}

void AnythingQuery::run(void *ptr, PushItemCallBack callBack, void *pdata)
{
    Q_ASSERT(callBack);

    AnythingQuery *self = static_cast<AnythingQuery *>(ptr);
    Q_ASSERT(self);

    auto d = self->d;
    qDebug() << "query by lingmo anything" << d->m_entity.size();
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
    d->m_time.start();
    // 搜索user目录下文件
    if (!d->searchUserPath(callBack, pdata))
        return; //中断

    // 使用anything搜索
    if (useAnything) {
        if (!d->searchByAnything(callBack, pdata))
            return; //中断
    }

    qDebug() << "lingmo anything is finished spend:" << d->m_time.elapsed() << "found:" << d->m_count;;
}

void AnythingQuery::setEntity(const QList<SemanticEntity> &entity)
{
    d->m_entity.clear();

    for (const SemanticEntity &e : entity) {
        if (!e.album.isEmpty() || !e.author.isEmpty() || !e.duration.isEmpty()
                || !e.resolution.isEmpty())
            continue;
        d->m_entity.append(e);
    }
}

void AnythingQuery::setFileHandler(FileResultsHandler *handler)
{
    d->m_handler = handler;
}
