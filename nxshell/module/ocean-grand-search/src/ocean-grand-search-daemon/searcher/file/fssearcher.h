// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FSSEARCHER_H
#define FSSEARCHER_H

#include "searcher/searcher.h"

extern "C" {
#include "fsearch.h"
}

#include <QFuture>
#include <QTime>

namespace GrandSearch {

class FsSearcher : public Searcher
{
    Q_OBJECT
public:
    explicit FsSearcher(QObject *parent = nullptr);
    ~FsSearcher() Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    bool isActive() const Q_DECL_OVERRIDE;
    bool activate() Q_DECL_OVERRIDE;
    ProxyWorker *createWorker() const Q_DECL_OVERRIDE;
    bool action(const QString &action, const QString &item) Q_DECL_OVERRIDE;
    void asyncInitDataBase();
private:
    static void loadDataBase(FsSearcher *fs);
    static void updateDataBase(FsSearcher *fs);
    void saveDataBase(Database *db);
private:
    bool m_isInited = false;
    bool m_isLoading = false;
    QFuture<void> m_loadFuture;

    FsearchApplication *m_app = nullptr;
    mutable Database *m_databaseForUpdate = nullptr;
    mutable QTime m_updateTime;
    mutable bool m_isUpdating = false;
    mutable QFuture<void> m_updateFuture;

    QTime m_databaseSaveTime;   // 索引保存计时
    int m_lastSaveTime = 0;
};

}

#endif // FSSEARCHER_H
