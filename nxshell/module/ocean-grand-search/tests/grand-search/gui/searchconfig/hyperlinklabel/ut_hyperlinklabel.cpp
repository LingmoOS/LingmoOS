// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/searchconfig/hyperlinklabel/hyperlinklabel.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QDesktopServices>
#include <QMouseEvent>
#include <QtTest>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(HyperlinkLabelTest, constructor)
{
    QString leadinText("leadinText");
    QString hyperlink("hyperlink");
    QString endText("endText");

    HyperlinkLabel *w = new HyperlinkLabel(leadinText, hyperlink, endText);

    EXPECT_TRUE(w);
    EXPECT_EQ(leadinText, w->m_leadingText);
    EXPECT_EQ(hyperlink, w->m_hyperlink);
    EXPECT_EQ(endText, w->m_endText);

    delete w;
}

TEST(HyperlinkLabelTest, paintEvent)
{
    QString leadinText("leadinText");
    QString hyperlink("hyperlink");
    QString endText("endText");

    HyperlinkLabel w(leadinText, hyperlink, endText);

    QRect rect(0, 0, 100, 100);
    QRegion region(rect);

    stub_ext::StubExt stu;

    stu.set_lamda(&HyperlinkLabel::drawText, [&]() {
        return region;
    });

    QPaintEvent event(region);

    w.paintEvent(&event);
    EXPECT_EQ(w.m_hyperlinkRegion, region);
}

TEST(HyperlinkLabelTest, mouseReleaseEvent)
{
    QString leadinText("leadinText");
    QString hyperlink("hyperlink");
    QString endText("endText");

    HyperlinkLabel w(leadinText, hyperlink, endText);

    stub_ext::StubExt stu;

    bool ut_call_openUrl = false;
    stu.set_lamda(&QDesktopServices::openUrl, [&]() {
        ut_call_openUrl = true;
        return ut_call_openUrl;
    });

    QRect rect(0, 0, 100, 100);
    QRegion region(rect);
    w.m_hyperlinkRegion = region;

    QTest::mouseRelease(&w, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));

    EXPECT_TRUE(ut_call_openUrl);
}

TEST(HyperlinkLabelTest, drawText)
{
    QString leadinText("leadinText");
    QString hyperlink("hyperlink");
    QString endText("endText");

    HyperlinkLabel w(leadinText, hyperlink, endText);

    stub_ext::StubExt stu;

    int ut_displayLength = 2;
    stu.set_lamda(&HyperlinkLabel::displayLength, [&]() {
        return ut_displayLength;
    });

    QString drawText("abcd");
    int startX = 0;
    int startY = 0;
    int curRow = 0;

    QRegion result = w.drawText(drawText, startX, startY, curRow);
    EXPECT_FALSE(result.isEmpty());
}

TEST(HyperlinkLabelTest, displayLength)
{
    QString leadinText("leadinText");
    QString hyperlink("hyperlink");
    QString endText("endText");

    HyperlinkLabel w(leadinText, hyperlink, endText);

    int startX = 0;
    QString showText("abcd");

    w.setFixedWidth(100);

    int result = w.displayLength(startX, showText);
    EXPECT_EQ(result, showText.size());

    startX = 0;
    showText = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    result = w.displayLength(startX, showText);
    EXPECT_NE(result, showText.size());

    startX = 20;
    result = w.displayLength(startX, showText);
    EXPECT_NE(result, showText.size());



}
