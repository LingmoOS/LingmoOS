// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bubbletitlewidget.h"
#include "notifylistview.h"

#include <QDateTime>
#include <QTest>

#include <gtest/gtest.h>

class UT_BubbleTitleWidget : public testing::Test
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
        obj = new BubbleTitleWidget(nullptr, entity);
    }

    void TearDown() override
    {
        if (obj) {
            delete obj;
            obj = nullptr;
        }
    }

public:
    BubbleTitleWidget *obj = nullptr;
    EntityPtr entity;
};

TEST_F(UT_BubbleTitleWidget, coverageTest)
{
    NotifyListView notifyListView;
    obj->bubbleElements();
    obj->setParentView(&notifyListView);
    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(obj, &enterEvent);
    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(obj, &leaveEvent);

    QEvent focusInEvent(QEvent::FocusIn);
    QApplication::sendEvent(obj, &focusInEvent);
    QEvent focusOutEvent(QEvent::FocusOut);
    QApplication::sendEvent(obj, &focusOutEvent);
}

TEST_F(UT_BubbleTitleWidget, indexRowTest)
{
    obj->setIndexRow(10);
    EXPECT_TRUE(10 == obj->indexRow());
}
