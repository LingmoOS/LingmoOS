// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "matchcontroller_p.h"
#include "matchcontroller.h"
#include "gui/datadefine.h"
#include "business/config/searchconfig.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"
#include "utils/utils.h"

#include "interfaces/daemongrandsearchinterface.h"

using namespace GrandSearch;

MatchControllerPrivate::MatchControllerPrivate(MatchController *parent)
    : q_p(parent)
{
    m_daemonDbus = new DaemonGrandSearchInterface(this);

    m_enableBestMatch = SearchConfig::instance()->getConfig(GRANDSEARCH_CUSTOM_GROUP, GRANDSEARCH_CUSTOM_BESTMATCH, m_enableBestMatch).toBool();
    m_firstItemLimit = SearchConfig::instance()->getConfig(GRANDSEARCH_CUSTOM_GROUP, GRANDSEARCH_CUSTOM_BESTMATCH_FIRSTITEMLIMIT, m_firstItemLimit).toInt();
    m_firstWaitTime = SearchConfig::instance()->getConfig(GRANDSEARCH_CUSTOM_GROUP, GRANDSEARCH_CUSTOM_BESTMATCH_FIRSTWAITTIME, m_firstWaitTime).toInt();
    m_bestItemMaxCount = SearchConfig::instance()->getConfig(GRANDSEARCH_CUSTOM_GROUP, GRANDSEARCH_CUSTOM_BESTMATCH_MAXCOUNT, m_bestItemMaxCount).toInt();

    initConnect();
}

void MatchControllerPrivate::initConnect()
{
    connect(m_daemonDbus, &DaemonGrandSearchInterface::Matched, this, &MatchControllerPrivate::onMatched);
    connect(m_daemonDbus, &DaemonGrandSearchInterface::SearchCompleted, this, &MatchControllerPrivate::onSearchCompleted);
}

void MatchControllerPrivate::onMatched(const QString &missionId)
{
    if (missionId != m_missionId || m_missionId.isEmpty())
        return;

    // 获取数据解析
    qDebug() << QString("m_daemonDbus->MatchedBuffer begin missionId:%1").arg(m_missionId);
    QByteArray matchedBytes = m_daemonDbus->MatchedBuffer(m_missionId);
    qDebug() << QString("m_daemonDbus->MatchedBuffer end   missionId:%1").arg(m_missionId);

    QDataStream stream(&matchedBytes, QIODevice::ReadOnly);

    MatchedItemMap items;
    stream >> items;

    // 首次收到数据
    if (m_missionIdChanged) {
        m_missionIdChanged = false;

        int count = 0;
        for (const MatchedItems &itemValues : items.values()) {
            count += itemValues.count();
        }
        qDebug() << "first recive data count:" << count << "limit count:" << m_firstItemLimit;

        // 总数不够时，将数据放入到缓存中，并开启延迟定时器
        if (count < m_firstItemLimit) {
            Utils::updateItemsWeight(items, m_missionContent);
            for (const QString &groupName : items.keys()) {
                m_cacheItems[groupName].append(items.value(groupName));
            }
            connect(m_waitTimer.get(), &QTimer::timeout, this, &MatchControllerPrivate::sendCacheItems);
            m_waitTimer->start(m_firstWaitTime);

            return;
        } else {
            Utils::updateItemsWeight(items, m_missionContent);
            Utils::sortByWeight(items);
            if (m_enableBestMatch)
                Utils::packageBestMatch(items, m_bestItemMaxCount);
        }
    }

    if (m_waitTimer.get() && m_waitTimer->isActive()) {
        // 首次数据不够，正在缓存数据
        Utils::updateItemsWeight(items, m_missionContent);
        for (const QString &groupName : items.keys()) {
            m_cacheItems[groupName].append(items.value(groupName));
        }

        return;
    }

    // 非首次、非缓存数据，不使用动态排序策略，直接发送
    emit q_p->matchedResult(items);

    qDebug() << QString("matched %1 groups.").arg(items.size());
}

void MatchControllerPrivate::onSearchCompleted(const QString &missionId)
{
    if (missionId != m_missionId || m_missionId.isEmpty())
        return;

    sendCacheItems();

    //通知界面刷新，若界面有数据不做处理，否则就刷新
    emit q_p->searchCompleted();

    qDebug() << QString("search Completed.");
}

void MatchControllerPrivate::sendCacheItems()
{
    if (m_cacheItems.isEmpty())
        return;

    Utils::sortByWeight(m_cacheItems);
    if (m_enableBestMatch)
        Utils::packageBestMatch(m_cacheItems, m_bestItemMaxCount);
    emit q_p->matchedResult(m_cacheItems);
    qDebug() << QString("send cache items %1 groups.").arg(m_cacheItems.size());
    m_cacheItems.clear();
}

MatchController::MatchController(QObject *parent)
    : QObject(parent)
    , d_p(new MatchControllerPrivate(this))
{

}

MatchController::~MatchController()
{

}

void MatchController::onMissionChanged(const QString &missionId, const QString &missionContent)
{
    if (Q_UNLIKELY(missionId == d_p->m_missionId))
        return;

    d_p->m_missionId = missionId;
    d_p->m_missionContent = missionContent;
    d_p->m_missionIdChanged = true;
    d_p->m_cacheItems.clear();
    if (d_p->m_waitTimer.get()) {
        d_p->m_waitTimer->disconnect();
    }
    d_p->m_waitTimer.reset(new QTimer);
    d_p->m_waitTimer->setSingleShot(true);
}

