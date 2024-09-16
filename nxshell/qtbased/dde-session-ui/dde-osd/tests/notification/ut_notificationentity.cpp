// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notification/notificationentity.h"

#include <QDateTime>

#include <gtest/gtest.h>

class UT_NotificationEntity : public testing::Test
{
public:
    void SetUp() override
    {
        obj = new NotificationEntity();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
    }

public:
    NotificationEntity *obj = nullptr;
};

TEST_F(UT_NotificationEntity, coverageTest)
{
    obj->setId("0");
    obj->setBody("图片已保存到/home/chenwei/Desktop/截图录屏_20200709104935.png");
    obj->setTime(QString::number(QDateTime::currentMSecsSinceEpoch()));
    obj->setHints(QVariantMap());
    obj->setActions(QStringList() << "_open" << "查看");
    obj->setAppIcon("deepin-screenshot");
    obj->setAppName("deepin-screen-recorder");
    obj->setIsTitle(false);
    obj->setSummary("截图完成");
    obj->setTimeout("-1");
    obj->setHideCount(0);
    obj->setReplacesId(QString::number(0));
    obj->setShowPreview(true);
    obj->setCurrentIndex(0);
    EXPECT_EQ(obj->currentIndex(), 0);
    obj->setShowInNotifyCenter(true);
    EXPECT_TRUE(obj->isShowInNotifyCenter());
}
