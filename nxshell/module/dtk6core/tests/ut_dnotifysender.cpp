// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#define private public
#include "dnotifysender.h"
#undef private

DCORE_USE_NAMESPACE

struct DUtil::DNotifyData {
    uint        m_replaceId;
    int         m_timeOut;
    QString     m_body;
    QString     m_summary;
    QString     m_appIcon;
    QString     m_appName;
    QStringList m_actions;
    QVariantMap m_hints;
};

TEST(ut_DNotifySender, notify)
{
    DUtil::DNotifySender notifySender("summary");
    notifySender.appName("appName")
            .replaceId(123456)
            .appIcon("iconName")
            .appBody("msg body")
            .actions({"1", "2"})
            .timeOut(5000);
            //.call();

    EXPECT_TRUE(notifySender.m_dbusData->m_summary == "summary");
    EXPECT_TRUE(notifySender.m_dbusData->m_body == "msg body");
    EXPECT_TRUE(notifySender.m_dbusData->m_replaceId == 123456);
    EXPECT_TRUE(notifySender.m_dbusData->m_appIcon == "iconName");
    EXPECT_TRUE(notifySender.m_dbusData->m_actions == QStringList({"1", "2"}));
    EXPECT_TRUE(notifySender.m_dbusData->m_timeOut == 5000);
}
