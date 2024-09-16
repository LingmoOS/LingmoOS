// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopappworker.h"
#include "global/builtinsearch.h"
#include "global/searchhelper.h"

using namespace GrandSearch;

DesktopAppWorker::DesktopAppWorker(const QString &name, QObject *parent) : ProxyWorker(name, parent)
{

}

void DesktopAppWorker::setContext(const QString &context)
{
    if (context.isEmpty())
        qWarning() << "search key is empty.";

    m_context = buildKeyword(context);
}

bool DesktopAppWorker::isAsync() const
{
    //同步搜索，需分配线程资源
    return false;
}

bool DesktopAppWorker::working(void *context)
{
    //准备状态切运行中，否则直接返回
    if (!m_status.testAndSetRelease(Ready, Runing))
        return false;

    Q_UNUSED(context);
    if (m_context.isEmpty() || m_indexTable.isEmpty()) {
        m_status.storeRelease(Completed);
        return true;
    }

    //计时
    QTime time;
    time.start();
    int lastEmit = 0;

    //遍历查找
    QHash<MatchedItem *, bool> found;
    for (auto iter = m_indexTable.begin(); iter != m_indexTable.end(); ++iter) {
        //中断
        if (m_status.loadAcquire() != Runing)
            return false;

        //匹配
        QRegExp regExp(m_context, Qt::CaseInsensitive);
        if (iter.key().contains(regExp)) {
            //遍历该关键词匹配的项目检查是否已经被添加
            for (const QSharedPointer<MatchedItem> &item : iter.value()) {
                //检查是否已经添加过
                if (found.contains(item.data()))
                    continue;

                //中断
                if (m_status.loadAcquire() != Runing)
                    return false;

                //加入搜索结果中
                {
                    QMutexLocker lk(&m_mtx);
                    m_items.append(*item);
                    found.insert(item.data(), true);

                    //数据从无到有发送通知
                    //50ms推送一次
                    int cur = time.elapsed();
                    if ((cur - lastEmit) > 50) {
                        lastEmit = cur;
                        qDebug() << "unearthed, current spend:" << cur;

                        //发送信号前必须解锁
                        lk.unlock();
                        emit unearthed(this);
                    }
                }
            }
        }
    }

    m_status.storeRelease(Completed);

    int leave = 0;
    {
        QMutexLocker lk(&m_mtx);
        leave = m_items.size();
    }

    qInfo() << "search completed, found items:" << found.size() << "total spend:" << time.elapsed()
            << "current items" << leave;
    //还有数据再发一次
    if (leave > 0)
        emit unearthed(this);

    return true;
}

void DesktopAppWorker::terminate()
{
    m_status.storeRelease(Terminated);
}

ProxyWorker::Status DesktopAppWorker::status()
{
    return static_cast<ProxyWorker::Status>(m_status.loadAcquire());
}

bool DesktopAppWorker::hasItem() const
{
    QMutexLocker lk(&m_mtx);
    return !m_items.isEmpty();
}

MatchedItemMap DesktopAppWorker::takeAll()
{
    QMutexLocker lk(&m_mtx);
    MatchedItems items = std::move(m_items);

    Q_ASSERT(m_items.isEmpty());
    lk.unlock();

    //添加分组
    MatchedItemMap ret;
    ret.insert(group(), items);

    return ret;
}

void DesktopAppWorker::setIndexTable(const QHash<QString, QList<QSharedPointer<MatchedItem> > > &table)
{
    qDebug() << "index table count" << table.size();
    //搜索中不允许设置索引表, 已有数据后不允许更新
    if (m_status.loadAcquire() == Runing || !m_indexTable.isEmpty())
        return;

    m_indexTable = table;
}

QString DesktopAppWorker::group() const
{
    return GRANDSEARCH_GROUP_APP;
}

QString DesktopAppWorker::buildKeyword(const QString &keyword)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(keyword.toLocal8Bit(), &error);
    if (error.error != QJsonParseError::NoError || doc.isEmpty())
        return searchHelper->tropeInputSymbol(keyword);

    // 应用搜索类目，只需要获取关键字信息
    QStringList keywordList;
    QJsonObject obj = doc.object();
    QJsonArray arr = obj[JSON_KEYWORD_ATTRIBUTE].toArray();
    for (int i = 0; i < arr.count(); ++i) {
        const QString &key = arr[i].toString();
        if (key.isEmpty())
            continue;
        keywordList.append(searchHelper->tropeInputSymbol(key));
    }

    // 搜索所有应用
    if (keywordList.isEmpty())
        return ".*";

    return keywordList.join('|');
}
