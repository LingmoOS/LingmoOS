// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "fssearcher.h"
#include "fsworker.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"

#include <QtConcurrent>

using namespace GrandSearch;

#define UPDATE_TIME_THRESHOLD   10*1000     // 索引更新阈值
#define DB_SAVE_INTERVAL        10*60*1000  // 索引存储时间

FsSearcher::FsSearcher(QObject *parent) : Searcher(parent)
{

}

FsSearcher::~FsSearcher()
{
    if (m_isLoading) {
        m_isLoading = false;

        m_loadFuture.waitForFinished();
    }

    if (m_isUpdating)
        m_updateFuture.waitForFinished();

    if (m_app) {
        if (m_app->db) {
            // 现目前重启/注销时，索引会保存失败将冲掉已有索引文件，因此暂时屏蔽
//            db_save_locations(m_app->db);
            db_clear(m_app->db);
            db_free(m_app->db);
        }

        if (m_app->pool)
            fsearch_thread_pool_free(m_app->pool);
        config_free(m_app->config);
        db_search_free(m_app->search);
        g_mutex_clear(&m_app->mutex);
        free(m_app);
        m_app = nullptr;
    }

    if (m_databaseForUpdate) {
        db_clear(m_databaseForUpdate);
        db_free(m_databaseForUpdate);
    }
}

QString FsSearcher::name() const
{
    return GRANDSEARCH_CLASS_FILE_FSEARCH;
}

bool FsSearcher::isActive() const
{
    return m_isInited;
}

bool FsSearcher::activate()
{
    return false;
}

ProxyWorker *FsSearcher::createWorker() const
{
    if (!m_isInited)
        return nullptr;

    // 通过数据库时间戳来确定当前是否需要替换数据库
    if ((m_databaseForUpdate->timestamp > m_app->db->timestamp) && !m_isUpdating)
        qSwap(m_app->db, m_databaseForUpdate);

    // 索引更新
    if (m_updateTime.elapsed() > UPDATE_TIME_THRESHOLD && !m_isUpdating) {
        m_isUpdating = true;
        m_updateFuture = QtConcurrent::run(updateDataBase, const_cast<FsSearcher *>(this));
    }

    auto worker = new FsWorker(name());
    worker->setFsearchApp(m_app);

    return worker;
}

bool FsSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item)
    qWarning() << "no such action:" << action << ".";
    return false;
}

void FsSearcher::asyncInitDataBase()
{
    if (m_isInited || m_isLoading)
        return;

    m_isLoading = true;
    m_loadFuture = QtConcurrent::run(loadDataBase, this);
}

void FsSearcher::loadDataBase(FsSearcher *fs)
{
    // 计时
    fs->m_updateTime.start();

    fs->m_app = static_cast<FsearchApplication *>(calloc(1, sizeof(FsearchApplication)));
    fs->m_app->config = static_cast<FsearchConfig *>(calloc(1, sizeof(FsearchConfig)));
    config_load_default(fs->m_app->config);
    fs->m_app->search = nullptr;
    fs->m_app->config->locations = nullptr;
    g_mutex_init(&fs->m_app->mutex);

    // 设置应用名，用于设置索引保存路径
    const auto &appName = QCoreApplication::organizationName() + "/" + GRANDSEARCH_DAEMON_NAME;
    g_set_application_name(appName.toLocal8Bit().data());

    // 索引/home/user目录
    QString searPath = QDir::homePath();
    fs->m_app->config->locations = g_list_append(fs->m_app->config->locations, searPath.toLocal8Bit().data());
    load_database(&fs->m_app->db, searPath.toLocal8Bit().data());
    load_database(&fs->m_databaseForUpdate, searPath.toLocal8Bit().data());

    fs->m_app->pool = fsearch_thread_pool_init();
    fs->m_app->search = db_search_new(fsearch_application_get_thread_pool(fs->m_app));

    fs->m_isInited = true;
    fs->m_isLoading = false;
    fs->m_databaseSaveTime.start();
    qInfo() << "load database complete,total items" << db_get_num_entries(fs->m_app->db) << "total spend" << fs->m_updateTime.elapsed();
}

void FsSearcher::updateDataBase(FsSearcher *fs)
{
    QTime time;
    time.start();

    fs->m_isUpdating = true;
    QString searPath = QDir::homePath();
    load_database(&fs->m_databaseForUpdate, searPath.toLocal8Bit().data());

    fs->saveDataBase(fs->m_databaseForUpdate);
    qInfo() << "update database complete,total spend" << time.elapsed();
    fs->m_isUpdating = false;
    fs->m_updateTime.restart();
}

void FsSearcher::saveDataBase(Database *db)
{
    int cur = m_databaseSaveTime.elapsed();
    if (cur - m_lastSaveTime > DB_SAVE_INTERVAL) {
        bool ret = db_save_locations(db);
        qDebug() << "database has saved: " << ret;
        m_lastSaveTime = cur;
    }
}
