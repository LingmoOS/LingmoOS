// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSWORKER_H
#define FSWORKER_H

#include "searcher/proxyworker.h"
#include "filesearchutils.h"

extern "C" {
#include "fsearch.h"
}

namespace GrandSearch {

class FsWorker : public ProxyWorker
{
public:
    explicit FsWorker(const QString &name, QObject *parent = nullptr);

    void setContext(const QString &context) Q_DECL_OVERRIDE;
    bool isAsync() const Q_DECL_OVERRIDE;
    bool working(void *context) Q_DECL_OVERRIDE;
    void terminate() Q_DECL_OVERRIDE;
    Status status() Q_DECL_OVERRIDE;
    bool hasItem() const Q_DECL_OVERRIDE;
    MatchedItemMap takeAll() Q_DECL_OVERRIDE;
    void setFsearchApp(FsearchApplication *app);
private:
    void initConfig();
    static void callbackReceiveResults(void *data, void *sender);
    bool appendSearchResult(const QString &fileName);
    bool searchLocalFile();
    void tryNotify();
    int itemCount() const;
    bool isResultLimit();
private:
    FsearchApplication *m_app = nullptr;
    QAtomicInt m_status = Ready;
    FileSearchUtils::SearchInfo m_searchInfo;                  // 搜索信息

    //搜索结果
    mutable QMutex m_mtx;
    MatchedItems m_items[FileSearchUtils::GroupCount];  // 文件搜索
    QHash<FileSearchUtils::Group, quint32> m_resultCountHash; // 记录各类型文件搜索结果数量

    QWaitCondition m_waitCondition;
    QMutex m_conditionMtx;
    QSet<QString> m_tmpSearchResults;     // 存储所有的搜索结果，用于去重
    QHash<QString, QSet<QString>> m_hiddenFilters;

    QTime m_time;       //搜索计时
    int m_lastEmit = 0;
};

}

#endif // FSWORKER_H
