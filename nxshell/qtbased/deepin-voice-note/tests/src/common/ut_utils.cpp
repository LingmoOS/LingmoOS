// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_utils.h"
#include "utils.h"
#include "datatypedef.h"
#include "vnoteitem.h"

#include <DStyle>
#include <DApplicationHelper>

UT_Utils::UT_Utils()
{
}

void UT_Utils::SetUp()
{
    m_utils = new Utils;
}

void UT_Utils::TearDown()
{
    delete m_utils;
}

TEST_F(UT_Utils, UT_Utils_convertDateTime_001)
{
    QDateTime currDateTime = QDateTime::currentDateTime();
    currDateTime = currDateTime.addSecs(-10);
    EXPECT_EQ(DApplication::translate("Utils", "1 min ago"), m_utils->convertDateTime(currDateTime));
    EXPECT_EQ(DApplication::translate("Utils", "2 mins ago"), m_utils->convertDateTime(currDateTime.addSecs(-120)));
    currDateTime = currDateTime.addSecs(-3600);
    EXPECT_EQ(currDateTime.toString("hh:mm"), m_utils->convertDateTime(currDateTime));
    currDateTime = currDateTime.addDays(-1);
    EXPECT_EQ(DApplication::translate("Utils", "Yesterday") + " " + currDateTime.toString("hh:mm"),
              m_utils->convertDateTime(currDateTime));
    for (int i = 1; i <= 777; i++) {
        QDateTime dateTime = currDateTime.addDays(-1 * i);
        if (QDate::currentDate().year() == dateTime.date().year()) {
            EXPECT_EQ(dateTime.toString("MM-dd"), m_utils->convertDateTime(dateTime)) << i << " days age";
        } else {
            EXPECT_EQ(dateTime.toString("yyyy-MM-dd"), m_utils->convertDateTime(dateTime)) << i << " days age";
        }
    }
}

TEST_F(UT_Utils, UT_Utils_renderSVG_001)
{
    QString fileName = "/tmp/play.svg";
    EXPECT_TRUE(m_utils->renderSVG(fileName, QSize(), qApp).isNull());

    EXPECT_TRUE(m_utils->renderSVG("play.svg", QSize(), qApp).isNull());
}

TEST_F(UT_Utils, UT_Utils_loadSVG_001)
{
    QString fileName = "play.svg";
    EXPECT_TRUE(m_utils->loadSVG(fileName, false).isNull()) << "fCommon = false";
    EXPECT_FALSE(m_utils->loadSVG(fileName, true).isNull()) << "fCommon = true";
}

TEST_F(UT_Utils, UT_Utils_highTextEdit_001)
{
    QTextDocument document;
    document.setPlainText("testeee");
    QString searchKey = "test";
    DPalette pb;
    QColor highColor = pb.color(DPalette::Highlight);
    EXPECT_EQ(1, m_utils->highTextEdit(&document, searchKey, highColor)) << "searchKey is 'test'";
    EXPECT_EQ(2, m_utils->highTextEdit(&document, "t", highColor, true)) << "searchKey is 't'";
    EXPECT_EQ(4, m_utils->highTextEdit(&document, "e", highColor, true)) << "searchKey is 'e'";
}

TEST_F(UT_Utils, UT_Utils_setDefaultColor_001)
{
    QTextDocument document;
    DPalette pb;
    QColor highColor = pb.color(DPalette::Highlight);
    m_utils->setDefaultColor(&document, highColor);
}

TEST_F(UT_Utils, UT_Utils_blockToDocument_001)
{
    VNTextBlock block;
    QTextDocument document;
    m_utils->blockToDocument(&block, &document);
}

TEST_F(UT_Utils, UT_Utils_documentToBlock_001)
{
    VNTextBlock block;
    QTextDocument document;
    block.blockText = "abc";
    document.setPlainText("test");
    m_utils->documentToBlock(&block, &document);
    EXPECT_EQ(document.toPlainText(), block.blockText);
}

TEST_F(UT_Utils, UT_Utils_formatMillisecond_001)
{
    qint64 tmpvoicesize = 890;
    m_utils->formatMillisecond(tmpvoicesize, 4);
    EXPECT_EQ(QDateTime::fromTime_t(static_cast<uint>(4)).toUTC().toString("mm:ss"), m_utils->formatMillisecond(tmpvoicesize, 4));
    for (int i = 5301; i < 7300000; i *= 1.5) {
        if (i < 3600000) {
            EXPECT_EQ(QDateTime::fromTime_t(static_cast<uint>(i / 1000)).toUTC().toString("mm:ss"), m_utils->formatMillisecond(i, 4));
        } else {
            EXPECT_EQ("60:00", m_utils->formatMillisecond(i, 4));
        }
    }
    tmpvoicesize = 18901111;
    EXPECT_EQ("60:00", m_utils->formatMillisecond(tmpvoicesize));
}

TEST_F(UT_Utils, UT_Utils_pictureToBase64_001)
{
    QString base64;
    EXPECT_FALSE(Utils::pictureToBase64("1.jpg", base64));
}

TEST_F(UT_Utils, UT_Utils_isLoongsonPlatform_001)
{
    EXPECT_FALSE(Utils::isLoongsonPlatform());
}

TEST_F(UT_Utils, UT_Utils_filteredFileName_001)
{
    EXPECT_EQ("123", Utils::filteredFileName("123"));
    EXPECT_EQ("123", Utils::filteredFileName("123 \"'/\\[]:|<>+=;,?* "));
    EXPECT_EQ("", Utils::filteredFileName("\"'/[]:|<  >+=;,?* "));
    EXPECT_EQ("note", Utils::filteredFileName("\"'/[]:|<  >+=;,?* ", "note"));
    EXPECT_EQ("qs", Utils::filteredFileName("\\q/s"));
    EXPECT_EQ("d saf  sa", Utils::filteredFileName("d saf / sa"));
}
