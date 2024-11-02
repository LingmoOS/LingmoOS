/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 */

#include "index-monitor.h"
#include <QRemoteObjectNode>
#include "rep_monitor_replica.h"

static LingmoUISearch::IndexMonitor *globalInstance = nullptr;
namespace LingmoUISearch {
class IndexMonitorPrivate {
public:
    ~IndexMonitorPrivate();
    QRemoteObjectNode m_qroNode;
    MonitorReplica *m_monitorReplica = nullptr;
    static QUrl nodeUrl();
};

IndexMonitorPrivate::~IndexMonitorPrivate()
{
    if(m_monitorReplica) {
        m_monitorReplica->disconnect();
        delete m_monitorReplica;
        m_monitorReplica = nullptr;
    }
}

QUrl IndexMonitorPrivate::nodeUrl()
{
    QString displayEnv = (qgetenv("XDG_SESSION_TYPE") == "wayland") ? QLatin1String("WAYLAND_DISPLAY") : QLatin1String("DISPLAY");
    QString display(qgetenv(displayEnv.toUtf8().data()));
    return QUrl(QStringLiteral("local:lingmo-search-service-monitor-") + QString(qgetenv("USER")) + display);
}

IndexMonitor *IndexMonitor::self()
{
    if(!globalInstance)
        globalInstance = new IndexMonitor;
    return globalInstance;
}

void IndexMonitor::stopMonitor()
{
    globalInstance->deleteLater();
    globalInstance = nullptr;
}

void IndexMonitor::startMonitor()
{
    IndexMonitor::self();
}

IndexMonitor::IndexMonitor(QObject *parent) : QObject(parent), d(new IndexMonitorPrivate)
{
    d->m_qroNode.connectToNode(IndexMonitorPrivate::nodeUrl());
    d->m_monitorReplica = d->m_qroNode.acquire<MonitorReplica>();
    //转发信号
    connect(d->m_monitorReplica, &MonitorReplica::currentIndexPathsChanged, this, &IndexMonitor::currentIndexPathsChanged);
    connect(d->m_monitorReplica, &MonitorReplica::indexStateChanged, this, &IndexMonitor::indexStateChanged);
    connect(d->m_monitorReplica, &MonitorReplica::basicIndexSizeChanged, this, &IndexMonitor::basicIndexSizeChanged);
    connect(d->m_monitorReplica, &MonitorReplica::contentIndexSizeChanged, this, &IndexMonitor::contentIndexSizeChanged);
    connect(d->m_monitorReplica, &MonitorReplica::ocrContentIndexSizeChanged, this, &IndexMonitor::ocrContentIndexSizeChanged);
    connect(d->m_monitorReplica, &MonitorReplica::aiIndexSizeChanged, this, &IndexMonitor::aiIndexSizeChanged);
    connect(d->m_monitorReplica, &MonitorReplica::basicIndexProgressChanged, this, &IndexMonitor::basicIndexProgressChanged);
    connect(d->m_monitorReplica, &MonitorReplica::contentIndexProgressChanged, this, &IndexMonitor::contentIndexProgressChanged);
    connect(d->m_monitorReplica, &MonitorReplica::ocrContentIndexProgressChanged, this, &IndexMonitor::ocrContentIndexProgressChanged);
    connect(d->m_monitorReplica, &MonitorReplica::aiIndexProgressChanged, this, &IndexMonitor::aiIndexProgressChanged);
    connect(d->m_monitorReplica, &MonitorReplica::basicIndexDocNumChanged, this, &IndexMonitor::basicIndexDocNumChanged);
    connect(d->m_monitorReplica, &MonitorReplica::contentIndexDocNumChanged, this, &IndexMonitor::contentIndexDocNumChanged);
    connect(d->m_monitorReplica, &MonitorReplica::ocrContentIndexDocNumChanged, this, &IndexMonitor::ocrContentIndexDocNumChanged);
    connect(d->m_monitorReplica, &MonitorReplica::aiIndexDocNumChanged, this, &IndexMonitor::aiIndexDocNumChanged);
    connect(d->m_monitorReplica, &MonitorReplica::basicIndexStart, this, &IndexMonitor::basicIndexStart);
    connect(d->m_monitorReplica, &MonitorReplica::contentIndexStart, this, &IndexMonitor::contentIndexStart);
    connect(d->m_monitorReplica, &MonitorReplica::ocrContentIndexStart, this, &IndexMonitor::ocrContentIndexStart);
    connect(d->m_monitorReplica, &MonitorReplica::aiIndexStart, this, &IndexMonitor::aiIndexStart);
    connect(d->m_monitorReplica, &MonitorReplica::basicIndexDone, this, &IndexMonitor::basicIndexDone);
    connect(d->m_monitorReplica, &MonitorReplica::contentIndexDone, this, &IndexMonitor::contentIndexDone);
    connect(d->m_monitorReplica, &MonitorReplica::ocrContentIndexDone, this, &IndexMonitor::ocrContentIndexDone);
    connect(d->m_monitorReplica, &MonitorReplica::aiIndexDone, this, &IndexMonitor::aiIndexDone);
    connect(d->m_monitorReplica, &MonitorReplica::basicIndexUpdatingChanged, this, &IndexMonitor::basicIndexUpdatingChanged);
    connect(d->m_monitorReplica, &MonitorReplica::contentIndexUpdatingChanged, this, &IndexMonitor::contentIndexUpdatingChanged);
    connect(d->m_monitorReplica, &MonitorReplica::ocrContentIndexUpdatingChanged, this, &IndexMonitor::ocrContentIndexUpdatingChanged);
    connect(d->m_monitorReplica, &MonitorReplica::aiIndexUpdatingChanged, this, &IndexMonitor::aiIndexUpdatingChanged);

    connect(d->m_monitorReplica, &QRemoteObjectReplica::initialized, this, &IndexMonitor::serviceReady);
    connect(d->m_monitorReplica, &QRemoteObjectReplica::stateChanged, this, [&](QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState){
        if(state == QRemoteObjectReplica::State::Suspect && oldState == QRemoteObjectReplica::State::Valid) {
            Q_EMIT serviceOffline();
        }
    });
}

IndexMonitor::~IndexMonitor()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

QStringList IndexMonitor::currentIndexPaths() const
{
    return d->m_monitorReplica->currentIndexPaths();
}

QString IndexMonitor::indexState() const
{
    return d->m_monitorReplica->indexState();
}

uint IndexMonitor::basicIndexSize() const
{
    return d->m_monitorReplica->basicIndexSize();
}

uint IndexMonitor::contentIndexSize() const
{
    return d->m_monitorReplica->contentIndexSize();
}

uint IndexMonitor::ocrContentIndexSize() const
{
    return d->m_monitorReplica->ocrContentIndexSize();
}

uint IndexMonitor::aiIndexSize() const
{
    return d->m_monitorReplica->aiIndexSize();
}

uint IndexMonitor::basicIndexProgress() const
{
    return d->m_monitorReplica->basicIndexProgress();
}

uint IndexMonitor::contentIndexProgress() const
{
    return d->m_monitorReplica->contentIndexProgress();
}

uint IndexMonitor::ocrContentIndexProgress() const
{
    return d->m_monitorReplica->ocrContentIndexProgress();
}

uint IndexMonitor::aiIndexProgress() const
{
    return d->m_monitorReplica->aiIndexProgress();
}

uint IndexMonitor::basicIndexDocNum() const
{
    return d->m_monitorReplica->basicIndexDocNum();
}

uint IndexMonitor::contentIndexDocNum() const
{
    return d->m_monitorReplica->contentIndexDocNum();
}

uint IndexMonitor::ocrContentIndexDocNum() const
{
    return d->m_monitorReplica->ocrContentIndexDocNum();
}

bool IndexMonitor::basicIndexUpdating() const
{
    return d->m_monitorReplica->basicIndexUpdating();
}

bool IndexMonitor::contentIndexUpdating() const
{
    return d->m_monitorReplica->contentIndexUpdating();
}

bool IndexMonitor::ocrContentIndexUpdating() const
{
    return d->m_monitorReplica->ocrContentIndexUpdating();
}

bool IndexMonitor::aiIndexUpdating() const
{
    return d->m_monitorReplica->aiIndexUpdating();
}
} // LingmoUISearch