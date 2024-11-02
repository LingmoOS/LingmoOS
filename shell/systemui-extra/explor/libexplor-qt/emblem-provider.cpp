/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "emblem-provider.h"
#include "metadata-emblem-provider.h"

#include "file-info.h"
#include "emblem-job.h"

#include <QTimer>
#include <QtConcurrent>
#include <QThreadPool>
#include <QRunnable>

#include <QCoreApplication>

#define QUERY_BATCH 500

using namespace Peony;

static EmblemProviderManager *global_instance = nullptr;
static QThreadPool *emblem_thread_pool = nullptr;

EmblemProvider::EmblemProvider(QObject *parent) : QObject(parent)
{

}

const QString EmblemProvider::emblemKey()
{
    Q_ASSERT_X(true, "emblemKey()", "This function should be reimplement");
    return metaObject()->className();
}

QStringList EmblemProvider::getFileEmblemIcons(const QString &uri)
{
    Q_UNUSED (uri)
    return QStringList();
}

EmblemProviderManager *EmblemProviderManager::getInstance()
{
    if (!global_instance)
        global_instance = new EmblemProviderManager;
    return global_instance;
}

void EmblemProviderManager::registerProvider(EmblemProvider *provider)
{
    m_providers.append(provider);

    connect(provider, &EmblemProvider::requestUpdateFile, this, &EmblemProviderManager::requestUpdateFile);
    connect(provider, &EmblemProvider::requestUpdateAllFiles, this, &EmblemProviderManager::requestUpdateAllFiles);
    connect(provider, &EmblemProvider::visibleChanged, this, [=](bool visible){
        this->visibleChanged(provider->emblemKey().toUtf8().constData(), visible);
    });
    connect(this, &EmblemProviderManager::queueQueryFinished, this, &EmblemProviderManager::requestUpdateAllFiles);
}

void EmblemProviderManager::unregisterProvider(EmblemProvider *provider)
{
    if (m_providers.contains(provider)) {
        m_providers.removeOne(provider);
    }
}

QStringList EmblemProviderManager::getAllEmblemsForUri(const QString &uri)
{
    auto info = FileInfo::fromUri(uri);
    QStringList ret;
    for (auto provider : m_providers) {
        ret << info->property(provider->emblemKey().toUtf8().constData()).toStringList();
    }
    ret.removeDuplicates();
    return ret;
}

QStringList EmblemProviderManager::getEmblemsByNameForUri(const QString &uri, const QString &name)
{
    auto info = FileInfo::fromUri(uri);
    return info->property(name.toUtf8().constData()).toStringList();
}

void EmblemProviderManager::querySync(const QString &uri)
{
    auto info = FileInfo::fromUri(uri);
    for (auto provider : m_providers) {
        auto emblems = provider->getFileEmblemIcons(uri);
        if (!emblems.isEmpty()) {
            info->setProperty(provider->emblemKey().toUtf8().constData(), emblems);
        } else {
            info->setProperty(provider->emblemKey().toUtf8().constData(), QVariant());
        }
    }
    requestUpdateFile(uri);
}

void EmblemProviderManager::queryAsync(const QString &uri)
{
    m_mutex.lock();
    m_queryQueue.append(uri);
    int currentCount = m_queryQueue.count();
    m_mutex.unlock();
    // 避免出现queue过长的情况
    if (currentCount >= QUERY_BATCH) {
        m_timer->stop();
        queryInternal();
    } else {
        if (!m_timer->isActive()) {
            m_timer->start();
        } else {
            // waiting for next queue.
        }
    }
}

void EmblemProviderManager::cancelQuery(const QString &uri)
{
    m_mutex.lock();
    m_queryQueue.removeAll(uri);
    m_mutex.unlock();
}

EmblemProviderManager::EmblemProviderManager(QObject *parent) : QObject(parent)
{
    emblem_thread_pool = new QThreadPool();

    registerProvider(MetadataEmblemProvider::getInstance());

    m_timer = new QTimer(this);
    m_timer->setInterval(0);
    m_timer->setSingleShot(true);

    connect(this, &EmblemProviderManager::queueQueryFinished, this, &EmblemProviderManager::queryInternal);
    connect(m_timer, &QTimer::timeout, this, &EmblemProviderManager::queryInternal);

    connect(qApp, &QCoreApplication::aboutToQuit, this, [=]{
        m_timer->stop();
        m_mutex.lock();
        m_queryQueue.clear();
        m_mutex.unlock();
    });
}

void EmblemProviderManager::queryInternal()
{
    m_mutex.lock();
    bool isEmpty = m_queryQueue.isEmpty();
    m_mutex.unlock();
    if (isEmpty) {
        return;
    }

    m_mutex.lock();
    // 由于构造job时使用了swap，需要把此操作互斥
    auto job = new EmblemJob(m_queryQueue, this);
    m_mutex.unlock();
    emblem_thread_pool->start(job);
}
