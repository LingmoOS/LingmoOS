// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <gtest/gtest.h>

#include <QSignalSpy>

#define private public
#define protected public
#include "business/matchresult/matchcontroller.h"
#include "business/matchresult/matchcontroller_p.h"
#include "utils/utils.h"
#include "global/matcheditem.h"

#include "interfaces/daemongrandsearchinterface.h"

using namespace testing;
using namespace GrandSearch;

TEST(MatchController, constructor)
{
    MatchController *matchController = new MatchController;
    ASSERT_TRUE(matchController);
    ASSERT_TRUE(matchController->d_p);
    delete matchController;
}

TEST(MatchController, onMissionIdChanged)
{
    MatchController matchController;

    QString id("aaaa");
    QString content("test");
    matchController.onMissionChanged(id, content);

    EXPECT_EQ(id, matchController.d_p->m_missionId);
    EXPECT_EQ(content, matchController.d_p->m_missionContent);
    EXPECT_TRUE(matchController.d_p->m_missionIdChanged);
}

TEST(MatchControllerPrivate, constructor)
{
    bool isCall = false;
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(MatchControllerPrivate, initConnect), [&](){
        isCall = true;
    });

    MatchController matchController;
    ASSERT_TRUE(matchController.d_p);
    ASSERT_TRUE(matchController.d_p->m_daemonDbus);
    EXPECT_TRUE(isCall);
}

TEST(MatchControllerPrivate, initConnect)
{
    MatchController matchController;
    matchController.d_p->initConnect();

    bool isCall = false;
    QString reciveId;
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(MatchControllerPrivate, onMatched), [&](MatchControllerPrivate *obj, QString missionId){
        Q_UNUSED(obj)

        isCall = true;
        reciveId = missionId;
    });

    QString missionId("testId");
    matchController.d_p->m_daemonDbus->Matched(missionId);
    EXPECT_TRUE(isCall);
    EXPECT_EQ(reciveId, missionId);

    isCall = false;
    reciveId.clear();

    stu.set_lamda(ADDR(MatchControllerPrivate, onSearchCompleted), [&](MatchControllerPrivate *obj, QString missionId){
        Q_UNUSED(obj)

        isCall = true;
        reciveId = missionId;
    });

    matchController.d_p->m_daemonDbus->SearchCompleted(missionId);
    EXPECT_TRUE(isCall);
    EXPECT_EQ(reciveId, missionId);
}

TEST(MatchControllerPrivate, onMatched)
{
    stub_ext::StubExt stu;
    stu.set_lamda(&DaemonGrandSearchInterface::MatchedBuffer, [](){
        QDBusPendingReply<QByteArray> reply;;
        return reply;
    });

    MatchController matchController;

    QSignalSpy spy(&matchController, &MatchController::matchedResult);

    EXPECT_EQ(spy.count(), 0);
    matchController.d_p->m_missionId.clear();
    matchController.d_p->onMatched(QString());
    EXPECT_EQ(spy.count(), 0);

    QString missionId("testId");
    QString content("tstContent");
    matchController.onMissionChanged(missionId, content);
    matchController.d_p->m_firstItemLimit = 30;

    stu.set_lamda(ADDR(Utils, updateItemsWeight), [&](MatchedItemMap &map, const QString &content){
        Q_UNUSED(content)

        MatchedItem tstItem;
        MatchedItems items;
        items << tstItem;
        map.insert(QString("tstGroup"), items);
    });

    matchController.d_p->onMatched(missionId);
    matchController.d_p->m_waitTimer->stop();
    EXPECT_FALSE(matchController.d_p->m_cacheItems.isEmpty());
    EXPECT_EQ(spy.count(), 0);

    QString missionId2("testId2");
    matchController.onMissionChanged(missionId2, content);
    matchController.d_p->m_cacheItems.clear();
    matchController.d_p->m_firstWaitTime = 0;
    matchController.d_p->onMatched(missionId2);
    EXPECT_FALSE(matchController.d_p->m_cacheItems.isEmpty());
    EXPECT_EQ(spy.count(), 0);

    matchController.d_p->m_missionIdChanged = true;
    matchController.d_p->m_firstItemLimit = 30;
    matchController.d_p->onMatched(missionId2);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_TRUE(matchController.d_p->m_waitTimer->isActive());

    matchController.d_p->onMatched(missionId2);
    EXPECT_EQ(spy.count(), 0);
    EXPECT_FALSE(matchController.d_p->m_cacheItems.isEmpty());

    matchController.d_p->m_waitTimer->stop();
    matchController.d_p->onMatched(missionId2);
    EXPECT_EQ(spy.count(), 1);
}

TEST(MatchControllerPrivate, onSearchCompleted)
{
    bool reciveSig = false;
    MatchController matchController;
    QObject::connect(&matchController, &MatchController::searchCompleted, qApp, [&](){
        reciveSig = true;
    });

    bool calledSend = false;
    stub_ext::StubExt stu;
    stu.set_lamda(&MatchControllerPrivate::sendCacheItems, [&](){
        calledSend = true;
    });

    matchController.d_p->m_missionId.clear();
    matchController.d_p->onSearchCompleted(QString());
    EXPECT_FALSE(reciveSig);
    EXPECT_FALSE(calledSend);

    QString missionId("testId");
    matchController.d_p->m_missionId = missionId;
    matchController.d_p->onSearchCompleted(missionId);
    EXPECT_TRUE(reciveSig);
    EXPECT_TRUE(calledSend);
}
