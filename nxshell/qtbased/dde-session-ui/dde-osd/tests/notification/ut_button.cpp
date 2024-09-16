// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "notification/button.h"
#undef private

#include <DMenu>

#include <QAction>
#include <QTest>
#include <QSignalSpy>
#include <QTimer>
#include <QApplication>

#include <gtest/gtest.h>

class UT_Button : public testing::Test
{
public:
    void SetUp() override
    {
        obj = new Button();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
    }

public:
    Button *obj = nullptr;
};

TEST_F(UT_Button, coverageTest)
{
    obj->show();
    obj->m_button->show();
    obj->m_menuArea->show();
    QTest::qWait(100);

    QTest::mousePress(obj->m_button, Qt::LeftButton);
    QTest::mouseRelease(obj->m_button, Qt::LeftButton);

    QSignalSpy spy1(obj, SIGNAL(clicked(bool checked)));
    QTimer::singleShot(100, [ = ] {
        if (obj && obj->m_menu)
            obj->m_menu->close();
    });
    QTest::mousePress(obj->m_menuArea, Qt::LeftButton);
    QTest::mouseRelease(obj->m_menuArea, Qt::LeftButton);
}

TEST_F(UT_Button, valueTest)
{
    obj->setRadius(10);
    EXPECT_TRUE(10 == obj->radius());

    obj->setId("Test");
    EXPECT_TRUE("Test" == obj->id());

    obj->setText("Test");
    EXPECT_TRUE("Test" == obj->text());

    obj->setTextAlignment(Qt::AlignTop);
    EXPECT_TRUE(Qt::AlignTop == obj->textAlignment());

    obj->setHoverState(true);
    EXPECT_TRUE(obj->boverState());

    QSize size(60, 50);
    EXPECT_TRUE(size == obj->sizeHint());

    QSize minisize(30, 25);
    EXPECT_TRUE(minisize == obj->minimumSizeHint());

    obj->setPixmap(QPixmap());
}

TEST_F(UT_Button, actionTest)
{
    QAction action(nullptr);
    obj->addAction(&action);
}
