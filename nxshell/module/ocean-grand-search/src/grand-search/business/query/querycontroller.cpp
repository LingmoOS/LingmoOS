// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "querycontroller_p.h"
#include "querycontroller.h"
#include "gui/datadefine.h"

#include "interfaces/daemongrandsearchinterface.h"

#include <QUuid>
#include <QDebug>

using namespace GrandSearch;

static const uint KeepAliveTime  = 15000;    // 搜索过程中，调用后端心跳函数间隔时间

QueryControllerPrivate::QueryControllerPrivate(QueryController *parent)
    : q_p(parent)
{
    m_keepAliveTimer = new QTimer(this);
    m_keepAliveTimer->setInterval(KeepAliveTime);

    m_daemonDbus = new DaemonGrandSearchInterface(this);

    // 前端启动时，通知后端停止可能的已有搜索任务，同时通过DBus调用保证后端已经启动
    m_daemonDbus->Terminate();

    connect(m_keepAliveTimer, &QTimer::timeout, this, &QueryControllerPrivate::keepAlive);
}

void QueryControllerPrivate::keepAlive()
{
    m_daemonDbus->KeepAlive(m_missionId);
}

void QueryControllerPrivate::onSearchCompleted(const QString &missionId)
{
    if (missionId == m_missionId) {
        m_keepAliveTimer->stop();
        qDebug() << "search completed and missionId:" << m_missionId;
    }
}

QueryController::QueryController(QObject *parent)
    : QObject(parent)
    , d_p(new QueryControllerPrivate(this))
{

}

QueryController::~QueryController()
{

}

void QueryController::onSearchTextChanged(const QString &txt)
{
    if (txt == d_p->m_searchText)
        return;

    d_p->m_keepAliveTimer->stop();

    if (txt.isEmpty()) {
        onTerminateSearch();
        qDebug() << "search terminate and missionId:" << d_p->m_missionId;
        d_p->m_searchText.clear();
        d_p->m_missionId.clear();

        // 发出搜索文本为空信号
        emit missionChanged(d_p->m_missionId, d_p->m_searchText);

        return;
    }

    d_p->m_searchText = txt;

    // 搜索文本改变，创建新的会话ID
    d_p->m_missionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    emit missionChanged(d_p->m_missionId, d_p->m_searchText);

    qDebug() << QString("m_daemonDbus->Search begin missionId:%1").arg(d_p->m_missionId);
    bool started = d_p->m_daemonDbus->Search(d_p->m_missionId, d_p->m_searchText);
    qDebug() << QString("m_daemonDbus->Search end   missionId:%1").arg(d_p->m_missionId);
    if (started) {
        d_p->m_keepAliveTimer->start();
    } else {
        qWarning() << QString("search failed missionId:%1").arg(d_p->m_missionId);
    }
}

void QueryController::onTerminateSearch()
{
    if (d_p->m_missionId.isEmpty())
        return;
    qDebug() << "m_daemonDbus->Terminate begin missionId:" << d_p->m_missionId;
    d_p->m_daemonDbus->Terminate();
    qDebug() << "m_daemonDbus->Terminate end   missionId:" << d_p->m_missionId;
}

QString QueryController::getMissionID() const
{
    return d_p->m_missionId;
}

bool QueryController::isEmptySearchText() const
{
    return d_p->m_searchText.isEmpty();
}
