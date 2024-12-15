// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/exhibition/preview/generalwidget/detailitem.h"
#include "gui/exhibition/preview/generalwidget/replicablelabel.h"
#include "gui/exhibition/preview/previewplugin.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>

#include <QVariant>
#include <QColor>
#include <QPaintEvent>
#include <QPainter>

using namespace testing;
using namespace GrandSearch;
DGUI_USE_NAMESPACE;

TEST(DetailItemTest, constructor)
{
    DetailItem *item = new DetailItem;

    ASSERT_TRUE(item);
    ASSERT_TRUE(item->m_tagLabel);
    ASSERT_TRUE(item->m_contentLabel);

    delete item;
}

TEST(DetailItemTest, setDetailInfo)
{
    DetailItem item;

    QString testKey("testKey");
    QString testVal("testVal");
    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant(testKey));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant(testVal));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
    item.setDetailInfo(detailInfo);

    QString getKey = item.m_tagLabel->text();
    EXPECT_EQ(getKey, testKey);

    QString getValue = item.m_contentLabel->m_fullText;
    EXPECT_EQ(getValue, testVal);
}

TEST(DetailItemTest, setTopRound)
{
    DetailItem item;

    bool topRound = false;
    item.setTopRound(topRound);
    EXPECT_FALSE(item.m_topRound);

    topRound = true;
    item.setTopRound(topRound);
    EXPECT_TRUE(item.m_topRound);
}

TEST(DetailItemTest, topRound)
{
    DetailItem item;

    item.m_topRound = false;
    bool getVal = item.topRound();
    EXPECT_FALSE(getVal);

    item.m_topRound = true;
    getVal = item.topRound();
    EXPECT_TRUE(getVal);
}

TEST(DetailItemTest, setBottomRound)
{
    DetailItem item;

    bool bottomRound = false;
    item.setBottomRound(bottomRound);
    EXPECT_FALSE(item.m_bottomRound);

    bottomRound = true;
    item.setBottomRound(bottomRound);
    EXPECT_TRUE(item.m_bottomRound);
}

TEST(DetailItemTest, bottomRound)
{
    DetailItem item;

    item.m_bottomRound = false;
    bool getVal = item.bottomRound();
    EXPECT_FALSE(getVal);

    item.m_bottomRound = true;
    getVal = item.bottomRound();
    EXPECT_TRUE(getVal);
}

TEST(DetailItemTest, setBackgroundColor)
{
    DetailItem item;

    QColor color(Qt::red);
    item.setBackgroundColor(color);
    QColor getVal = item.m_backgroundColor;
    EXPECT_EQ(getVal, color);
}

TEST(DetailItemTest, backgroundColor)
{
    DetailItem item;

    QColor color(Qt::red);
    item.m_backgroundColor = color;

    QColor getVal = item.backgroundColor();
    EXPECT_EQ(getVal, color);
}

TEST(DetailItemTest, setRadius)
{
    DetailItem item;

    int radius = 10;
    item.setRadius(radius);
    int getVal = item.m_radius;
    EXPECT_EQ(getVal, radius);
}

TEST(DetailItemTest, radius)
{
    DetailItem item;

    int radius = 10;
    item.m_radius = radius;
    int getVal = item.radius();
    EXPECT_EQ(getVal, radius);
}

TEST(DetailItemTest, setTagWidth)
{
    DetailItem item;

    int width = 10;
    item.setTagWidth(width);
    int getValue = item.m_tagLabel->width();
    EXPECT_EQ(width, getValue);
}

TEST(DetailItemTest, tagWidth)
{
    DetailItem item;

    int width = 10;
    item.m_tagLabel->setFixedWidth(width);
    int getVal = item.tagWidth();
    EXPECT_EQ(width, getVal);
}

TEST(DetailItemTest, paintEvent)
{
    DetailItem item;

    stub_ext::StubExt stu;

    bool ut_call_drawPath = false;
    stu.set_lamda(ADDR(QPainter, drawPath), [&](){
        ut_call_drawPath = true;
    });

    // 圆角
    item.m_topRound = true;
    item.m_bottomRound = true;

    QPaintEvent event(QRect(QPoint(10, 10), QSize(20, 20)));
    item.paintEvent(&event);

    EXPECT_TRUE(ut_call_drawPath);

    // 直角
    item.m_topRound = false;
    item.m_bottomRound = false;
    item.paintEvent(&event);

    EXPECT_TRUE(ut_call_drawPath);
}

TEST(DetailItemTest, getTagColor)
{
    DetailItem item;

    stub_ext::StubExt stu;

    DGuiApplicationHelper::ColorType ut_themeType = DGuiApplicationHelper::DarkType;
    stu.set_lamda(ADDR(DGuiApplicationHelper, themeType), [&](){
        return ut_themeType;
    });

    QColor color = item.getTagColor();
    EXPECT_TRUE(color.isValid());
}

TEST(DetailItemTest, getContentColor)
{
    DetailItem item;

    stub_ext::StubExt stu;

    DGuiApplicationHelper::ColorType ut_themeType = DGuiApplicationHelper::DarkType;
    stu.set_lamda(ADDR(DGuiApplicationHelper, themeType), [&](){
        return ut_themeType;
    });

    QColor color = item.getContentColor();
    EXPECT_TRUE(color.isValid());

}
