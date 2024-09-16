// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_customframe.h"

test_customframe::test_customframe()
{
    mCustomFrame = new CustomFrame();
}

test_customframe::~test_customframe()
{
    delete mCustomFrame;
    mCustomFrame = nullptr;
}

//void CustomFrame::setBColor(QColor normalC)
TEST_F(test_customframe, setBcolor)
{
    mCustomFrame->setBColor(Qt::red);
}

//void CustomFrame::setRoundState(bool lstate, bool tstate, bool rstate, bool bstate)
TEST_F(test_customframe, setRoundState)
{
    mCustomFrame->setRoundState(true, true, true, true);
}

//void CustomFrame::setTextStr(QFont font, QColor tc, QString strc, int flag)
TEST_F(test_customframe, setTextStr)
{
    QFont font;
    mCustomFrame->setTextStr(font, Qt::red, "hello", 1);
}

//void CustomFrame::setTextStr(QString strc)
TEST_F(test_customframe, setTextstr)
{
    mCustomFrame->setTextStr("hello");
}

//void CustomFrame::setTextColor(QColor tc)
TEST_F(test_customframe, setTextColor)
{
    mCustomFrame->setTextColor(Qt::red);
}

//void CustomFrame::setTextFont(QFont font)
TEST_F(test_customframe, setTextFont)
{
    QFont font;
    mCustomFrame->setTextFont(font);
}

//void CustomFrame::setTextAlign(int flag)
TEST_F(test_customframe, setTextAlign)
{
    mCustomFrame->setTextAlign(2);
}

//void CustomFrame::setRadius(int radius)
TEST_F(test_customframe, setRadius)
{
    mCustomFrame->setRadius(4);
}

//void CustomFrame::setboreder(int framew)
TEST_F(test_customframe, setboreder)
{
    mCustomFrame->setboreder(4);
}

//void CustomFrame::setFixedSize(int w, int h)
TEST_F(test_customframe, setFixedSize)
{
    mCustomFrame->setFixedSize(40, 44);
}

