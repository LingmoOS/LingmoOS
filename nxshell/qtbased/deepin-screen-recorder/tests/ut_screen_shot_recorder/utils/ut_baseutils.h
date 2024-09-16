// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QDebug>
#include "../../src/utils/baseutils.h"
#include <gtest/gtest.h>



using namespace testing;

class Baseutils_Test:public testing::Test{

public:
    virtual void SetUp() override{
        std::cout << "start UtilsTest" << std::endl;
    }

    virtual void TearDown() override{
        std::cout << "end UtilsTest" << std::endl;
    }
};

TEST_F(Baseutils_Test, colorIndexOf)
{
    QColor color = BaseUtils::colorIndexOf(0); // QColor("#fflc49")
    int a, r, g, b;

    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0xff, r);
    EXPECT_EQ(0x1c, g);
    EXPECT_EQ(0x49, b);

    color = BaseUtils::colorIndexOf(1); //  QColor("#ffd903")
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0xff, r);
    EXPECT_EQ(0xd9, g);
    EXPECT_EQ(0x03, b);

    color = BaseUtils::colorIndexOf(2); //  QColor("#ffd903")
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0x00, r);
    EXPECT_EQ(0x89, g);
    EXPECT_EQ(0xF7, b);


    color = BaseUtils::colorIndexOf(3); //  QColor("#ffd903")
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0x08, r);
    EXPECT_EQ(0xff, g);
    EXPECT_EQ(0x77, b);

    color = BaseUtils::colorIndexOf(4); //  QColor("#ffd903")
    color.getRgb(&r, &g, &b, &a);
    EXPECT_EQ(0xff, r);
    EXPECT_EQ(0x1c, g);
    EXPECT_EQ(0x49, b);
}

TEST_F(Baseutils_Test, colorIndex)
{
    int index = BaseUtils::colorIndex(QColor("#ff1c49"));
    EXPECT_EQ(0, index);

    index = BaseUtils::colorIndex(QColor("#ff3305"));
    EXPECT_EQ(5, index);
}
TEST_F(Baseutils_Test, isValidFormat)
{
    bool valid = BaseUtils::isValidFormat("bmp");
    EXPECT_TRUE(valid);

    valid = BaseUtils::isValidFormat("mp4");
    EXPECT_FALSE(valid);
}
/*
TEST_F(Baseutils_Test, getFileContent)
{
    const QString testValue = "0123456789abcdefg";
    const QString fileName = "testFile.conf";
    QString cmd = QString("echo '%1' >> %2").arg(testValue).arg(fileName);
    system("echo '0123456789abcdefg' >> testFile.conf");
    QString string = getFileContent(fileName);
    EXPECT_FALSE(string.isEmpty());
    EXPECT_TRUE(string.split("\n")[0] == testValue);
    system("rm testFile.conf");
}
*/
TEST_F(Baseutils_Test, stringWidth)
{
    const QString testValue = "0123456789abcdefg";
    const QString testValueCn = "统信UOS";
    QFont font("Arial");
    qDebug() << BaseUtils::stringWidth(font, testValue);
    //qDebug() << stringWidth(font, testValueCn);
    //EXPECT_EQ(17, stringWidth(font, testValue));
    //EXPECT_EQ(10, stringWidth(font, testValueCn));
}
TEST_F(Baseutils_Test, setCursorShape)
{
    QString cursorName = "start";
    int colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);

    cursorName = "rotate";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);


    cursorName = "rectangle";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);

    cursorName = "oval";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);


    cursorName = "arrow";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);

    cursorName = "text";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);

    cursorName = "line";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);

    cursorName = "straightLine";
    colorIndex = 0;
    BaseUtils::setCursorShape(cursorName, colorIndex);
}
