// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_imageviewerdialog.h"
#include "dialog/imageviewerdialog.h"

UT_Imageviewerdialog::UT_Imageviewerdialog()
{
}

TEST_F(UT_Imageviewerdialog, ut_imageviewerdialog_open_001)
{
    ImageViewerDialog image;
    image.open("/tmp/1.jpg");
}

TEST_F(UT_Imageviewerdialog, UT_Imageviewerdialog_initUI_001)
{
    ImageViewerDialog image;
    EXPECT_TRUE(image.m_imgLabel) << "m_imgLabel";
    EXPECT_EQ("ImageLabel", image.m_imgLabel->objectName()) << "m_imgLabel.objectName";
    EXPECT_TRUE(image.m_closeButton) << "m_closeButton";
    EXPECT_EQ("CloseButton", image.m_closeButton->objectName()) << "m_closeButton.objectName";
}

TEST_F(UT_Imageviewerdialog, ut_imageviewerdialog_paintEvent_001)
{
    ImageViewerDialog image;
    image.paintEvent(nullptr);
}
