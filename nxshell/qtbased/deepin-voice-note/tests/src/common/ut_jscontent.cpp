// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_jscontent.h"
#include "jscontent.h"

UT_JsContent::UT_JsContent()
{
}

TEST_F(UT_JsContent, UT_JsContent_instance_001)
{
    JsContent *instance = JsContent::instance();
    EXPECT_EQ(instance, JsContent::instance());
}

TEST_F(UT_JsContent, UT_JsContent_insertImages_001)
{
    JsContent *instance = JsContent::instance();
    EXPECT_FALSE(instance->insertImages(QStringList("/tmp/1.jpg"))) << "1.jpg";
    EXPECT_FALSE(instance->insertImages(QStringList("1.bmp"))) << "1.bmp";
    EXPECT_FALSE(instance->insertImages(QStringList("1.mp3"))) << "1.mp3";
}

TEST_F(UT_JsContent, UT_JsContent_insertImages_002)
{
    JsContent *instance = JsContent::instance();
    QImage image;
    EXPECT_FALSE(instance->insertImages(image));
}

TEST_F(UT_JsContent, UT_JsContent_jsCallTxtChange_001)
{
    JsContent::instance()->jsCallTxtChange();
}

TEST_F(UT_JsContent, UT_JsContent_jsCallChannleFinish_001)
{
    JsContent::instance()->jsCallChannleFinish();
}

TEST_F(UT_JsContent, UT_JsContent_jsCallPopupMenu_001)
{
    JsContent::instance()->jsCallPopupMenu(1, QVariant());
}

TEST_F(UT_JsContent, UT_JsContent_jsCallPlayVoice_001)
{
    JsContent::instance()->jsCallPlayVoice(QVariant(), true);
}

TEST_F(UT_JsContent, UT_JsContent_jsCallGetTranslation_001)
{
    EXPECT_FALSE(JsContent::instance()->jsCallGetTranslation().isEmpty());
}

TEST_F(UT_JsContent, UT_JsContent_callJsSynchronous_001)
{
    EXPECT_TRUE(JsContent::instance()->callJsSynchronous(nullptr, "").isNull());
}

TEST_F(UT_JsContent, UT_JsContent_jsCallSetDataFinsh_001)
{
    JsContent::instance()->jsCallSetDataFinsh();
}

TEST_F(UT_JsContent, UT_JsContent_jsCallPaste_001)
{
    JsContent::instance()->jsCallPaste();
}

TEST_F(UT_JsContent, UT_JsContent_jsCallViewPicture_001)
{
    JsContent::instance()->jsCallViewPicture("1.jpg");
}

TEST_F(UT_JsContent, UT_JsContent_jsCallCreateNote_001)
{
    JsContent::instance()->jsCallCreateNote();
}

TEST_F(UT_JsContent, UT_JsContent_jsCallSetClipData_001)
{
    JsContent::instance()->jsCallSetClipData("", "");
}
