// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EDIDParser.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_EDIDParser : public UT_HEAD
{
public:
    void SetUp()
    {
        m_EDIDParser = new EDIDParser;
    }
    void TearDown()
    {
        delete m_EDIDParser;
    }
    EDIDParser *m_EDIDParser = nullptr;
};

//bool setEdid(const QString &edid, QString &errorMsg, const QString &ch = "\n", bool littleEndianMode = true);
QString edid = "00ffffffffffff005a63384001010101\n"
               "0d1e010380351d782ece65a657519f27\n"
               "0f5054bfef80b300a940a9c095009040\n"
               "8180814081c0023a801871382d40582c\n"
               "45000f282100001e000000ff00565351\n"
               "3230313332313330320a000000fd0032\n"
               "4b185311000a202020202020000000fc\n"
               "005641323433302d4648440a20200141\n"
               "020320f14d9005040302121113141e1d\n"
               "1f0123097f078301000065030c001000\n"
               "023a801871382d40582c45000f282100\n"
               "001e011d8018711c1620582c25000f28\n"
               "2100009e011d007251d01e206e285500\n"
               "0f282100001e8c0ad08a20e02d10103e\n"
               "96000f28210000188c0ad09020403120\n"
               "0c4055000f28210000180000000000d6\n";
TEST_F(UT_EDIDParser,UT_EDIDParser_setEdid){
    QString errorMsg;
    EXPECT_TRUE(m_EDIDParser->setEdid(edid,errorMsg));
}

//const QString &vendor()const;
TEST_F(UT_EDIDParser,UT_EDIDParser_vendor){
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    EXPECT_STREQ("VSC",m_EDIDParser->vendor().toStdString().c_str());
}

//const QString &releaseDate()const;
TEST_F(UT_EDIDParser,UT_EDIDParser_releaseDate){
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    EXPECT_STREQ("2020-03",m_EDIDParser->releaseDate().toStdString().c_str());
}

//const QString &screenSize()const;
TEST_F(UT_EDIDParser,UT_EDIDParser_screenSize){
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    EXPECT_STREQ("23.8 inch(53mm X 29mm)",m_EDIDParser->screenSize().toStdString().c_str());
}

//int width();
TEST_F(UT_EDIDParser,UT_EDIDParser_width){
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    EXPECT_EQ(53,m_EDIDParser->width());
}

//int height();
TEST_F(UT_EDIDParser,UT_EDIDParser_height){
    QString errorMsg;
    m_EDIDParser->setEdid(edid,errorMsg);
    EXPECT_EQ(29,m_EDIDParser->height());
}

//QString binToDec(QString strBin);
TEST_F(UT_EDIDParser,UT_EDIDParser_binToDec){
    EXPECT_STREQ("15",m_EDIDParser->binToDec("00001111").toStdString().c_str());
}

//QString decTobin(QString strDec);
TEST_F(UT_EDIDParser,UT_EDIDParser_decTobin){
    EXPECT_STREQ("1111",m_EDIDParser->decTobin("15").toStdString().c_str());
}

//QString decToHex(QString strDec);
TEST_F(UT_EDIDParser,UT_EDIDParser_decToHex){
    EXPECT_STREQ("0000000f",m_EDIDParser->decToHex("15").toStdString().c_str());
}

