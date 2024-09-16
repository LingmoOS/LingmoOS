// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_exitdialog.h"

#include <DHiDPIHelper>
#include <QPaintEvent>

TEST_F(UT_ExitDialog, UT_ExitDialog_initUI)
{
    m_ExitDialog->initUI();
}

TEST_F(UT_ExitDialog, UT_ExitDialog_initBackgroundPix1)
{
    m_ExitDialog->compositingStatus = true;
    m_ExitDialog->initBackgroundPix();
    EXPECT_EQ(m_ExitDialog->backgroundQPixmap, DHiDPIHelper::loadNxPixmap(":/resources/exitdialog/close-dialog.svg"))
            << "check exitDialog initBackgroundPix";
}

TEST_F(UT_ExitDialog, UT_ExitDialog_initBackgroundPix2)
{
    m_ExitDialog->compositingStatus = false;
    m_ExitDialog->initBackgroundPix();
    EXPECT_EQ(m_ExitDialog->backgroundQPixmap, DHiDPIHelper::loadNxPixmap(":/resources/exitdialog/close-dialog-nshadow.svg"))
            << "check exitDialog initBackgroundPix";
}

TEST_F(UT_ExitDialog, UT_ExitDialog_slotDialogClose)
{
    m_ExitDialog->soltDialogClose();
}

TEST_F(UT_ExitDialog, UT_ExitDialog_soltGameExit)
{
    m_ExitDialog->soltGameExit();
}

TEST_F(UT_ExitDialog, UT_ExitDialog_slotCompositingChanged)
{
    m_ExitDialog->slotCompositingChanged(true);
    EXPECT_EQ(m_ExitDialog->compositingStatus, true)
            << "check exitDialog slotCompositingChanged";
}

TEST_F(UT_ExitDialog, UT_ExitDialog_paintEvent)
{
    QRect rect;
    QPaintEvent event(rect);
    m_ExitDialog->paintEvent(&event);
}
