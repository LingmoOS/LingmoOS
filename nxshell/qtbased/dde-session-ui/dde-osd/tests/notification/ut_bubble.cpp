// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "notification/bubble.h"
#include "notification/constants.h"
#undef private

#include <QtTest>

#include <gtest/gtest.h>

class UT_Bubble : public testing::Test
{
public:
    void SetUp() override
    {
        QString appName = "deepin-screen-recorder";
        uint id = 0;
        QString appIcon = "deepin-screenshot";
        QString summary = "截图完成";
        QString body = "图片已保存到/home/chenwei/Desktop/截图录屏_20200709104935.png";
        QStringList actions ;
        actions << "_open" << "查看";
        QVariantMap hints;
        int expireTimeout = -1;

        entity = std::make_shared<NotificationEntity>(appName, QString(), appIcon,
                                                                      summary, body, actions, hints,
                                                                      QString::number(QDateTime::currentMSecsSinceEpoch()),
                                                                      QString::number(id),
                                                                      QString::number(expireTimeout));
        obj = new Bubble(nullptr, entity);
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
    }

public:
    Bubble *obj = nullptr;
    EntityPtr entity;
};

TEST_F(UT_Bubble, coverageTest)
{
    obj->entity();
    obj->setEntity(entity);
    obj->updateGeometry();
    obj->setEnabled(true);
    obj->setEnabled(false);
    obj->setBubbleIndex(0);
    obj->initTimers();
    obj->bubbleIndex();
}

TEST_F(UT_Bubble, eventTest)
{
    // 测试鼠标点击并松开后是否发出dismissed信号
    qRegisterMetaType<Bubble *>("Bubble *");
    QSignalSpy spy(obj, SIGNAL(dismissed(Bubble *)));
    QTest::mousePress(obj, Qt::LeftButton, Qt::KeyboardModifiers(),QPoint(0, 0));
    QTest::mouseRelease(obj, Qt::LeftButton, Qt::KeyboardModifiers(),QPoint(0, 0));

    EXPECT_EQ(spy.count(), 1);

    // 测试鼠标点击并松开后，如果松开与点击的位置不同，是否发出notProcessedYet
    qRegisterMetaType<EntityPtr>("EntityPtr");
    QSignalSpy spy1(obj, SIGNAL(dismissed(Bubble *)));
    QTest::mousePress(obj, Qt::LeftButton, Qt::KeyboardModifiers(),QPoint(0, 0));
    QTest::mouseRelease(obj, Qt::LeftButton, Qt::KeyboardModifiers(),QPoint(0, 5));

    EXPECT_EQ(spy1.count(), 1);

    QEvent showEvent(QEvent::Show);
    QApplication::sendEvent(obj, &showEvent);

    QEvent hoverEvent(QEvent::Leave);
    QApplication::sendEvent(obj, &hoverEvent);

    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(obj, &enterEvent);

    QEvent hideEvent(QEvent::Hide);
    QApplication::sendEvent(obj, &hideEvent);
}
