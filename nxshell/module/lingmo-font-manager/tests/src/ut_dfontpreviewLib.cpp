// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreview.h"
#include "dfontwidget.h"

#include <QApplication>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QTextStream>
#include <QFileInfo>
#include <QPainter>
#include <QScreen>
#include <QDebug>
#include <QPaintEvent>
#include <QFile>
#include <QDir>

#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>
#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "commonheaderfile.h"

#define FTM_DEFAULT_PREVIEW_CN_TEXT QString("因理想而出生，为责任而成长")
#define FTM_DEFAULT_PREVIEW_EN_TEXT QString("Don't let your dreams be dreams")
#define FTM_DEFAULT_PREVIEW_DIGIT_TEXT QString("0123456789")

namespace {
class TestDFontPreview : public testing::Test
{

protected:
    void SetUp()
    {
        fp = new DFontPreview();
    }
    void TearDown()
    {
        SAFE_DELETE_ELE(fp)
    }
    // Some expensive resource shared by all tests.
    DFontPreview *fp;
};

static QString g_funcname;
QString stub_getSampleString()
{
    g_funcname = __FUNCTION__;
    return "SampleString";
}
// typedef struct FT_FaceRec_*  FT_Face;
FT_Error stub_FT_New_Face(FT_Library, const char *, FT_Long, FT_Face *aface)
{
    *aface = new FT_FaceRec_;
    (*aface)->style_name = new char[8];
    const char *tmp = "Regular";
    memcpy((*aface)->style_name, tmp, 8);
    return 0;
}
FT_Error stub_FT_Done_Face(FT_Face face)
{
    SAFE_DELETE_ARRAY(face->style_name)
    SAFE_DELETE_ELE(face)
    return 0;
}
}

TEST_F(TestDFontPreview, checkSetFileUrl)
{
    Stub s;
    s.set(ADDR(DFontPreview, getSampleString), stub_getSampleString);

    //传入错误参数
    g_funcname.clear();
    fp->setFileUrl("file:///usr/share/fonts/truetype/liberation/LiberationSans-Italic.ttf");
    EXPECT_FALSE(g_funcname == QLatin1String("stub_getSampleString"));

    s.set(FT_New_Face, stub_FT_New_Face);
    s.set(FT_Done_Face, stub_FT_Done_Face);
    //传入正常参数
    fp->setFileUrl("/usr/share/fonts/truetype/liberation/LiberationSans-Italic.ttf");
    EXPECT_TRUE(g_funcname == QLatin1String("stub_getSampleString"));
    SAFE_DELETE_ELE(fp)
}

TEST_F(TestDFontPreview, checkPaintEvent)
{

    QRect r;
    QString filepath = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    QByteArray fileContent = file.readAll();

    qDebug() << fp->fontDatabase.addApplicationFontFromData(fileContent);

    fp->setFileUrl(filepath);
    QPaintEvent *p = new QPaintEvent(r);
    fp->m_error = false;
    //不要滚动条
    fp->paintEvent(p);

    //要滚动条
    fp->currentMaxWidth = 1500;
    fp->paintEvent(p);

    SAFE_DELETE_ELE(p)
}

TEST_F(TestDFontPreview, checkGetLanguageSampleString)
{
    fp->m_contents.clear();
    fp->m_contents.insert("es", "first");
    //不存在的语言
    EXPECT_TRUE(fp->getLanguageSampleString("first").isEmpty());

    //构建特殊语言
    EXPECT_TRUE(fp->getLanguageSampleString("es_first") == "first");
}




