// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreviewer.h"
#include "dfontpreviewlistdatathread.h"
#include "utils.h"

#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QRawFont>
#include <QDir>

#include <DPalette>
#include <DApplicationHelper>
#include "dfmdbmanager.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "commonheaderfile.h"
#include "interfaces/dcomworker.h"

int cuntch = 0;

namespace {
class TestDFontPreviewer : public testing::Test
{

protected:
    void SetUp()
    {
        fm = new DFontPreviewer();
    }
    void TearDown()
    {
        delete fm;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFontPreviewer *fm;

public:
    bool stub_supportsCharacter(QChar character);
    bool stub_bool();

};

static QString g_funcname;
void stub_drawText(const QRect &, int, const QString &, QRect *)
{
    g_funcname = __FUNCTION__;
}
bool stub_remove()
{
    g_funcname = __FUNCTION__;
    return true;
}
bool stub_removeRecursively()
{
    return true;
}
QStringList stub_getInstalledFontsPath()
{
    QStringList installedList;
    installedList.append("/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf");
    installedList.append("/usr/share/fonts/truetype/lohit-devanagari/Lohit-Devanagari.ttf");
    return installedList;
}
}

TEST_F(TestDFontPreviewer, checkInitData)
{
    fm->InitData();
    EXPECT_FALSE(fm->m_previewTexts.isEmpty());
    EXPECT_TRUE(fm->m_previewTexts.contains("汉体书写信息技术标准相容"));
}

bool TestDFontPreviewer::stub_supportsCharacter(QChar character)
{
    if (cuntch == 0) {
        cuntch++;
        return false;
    } else {
        return true;
    }
}

bool TestDFontPreviewer::stub_bool()
{
    return false;
}


TEST_F(TestDFontPreviewer, checkOnPreviewFontChanged)
{
    fm->onPreviewFontChanged();
    EXPECT_FALSE(fm->m_previewTexts.isEmpty());
    Stub st;
    st.set((bool (QRawFont::*)(QChar character) const)ADDR(QRawFont, supportsCharacter), ADDR(TestDFontPreviewer, stub_supportsCharacter));
    cuntch = 0;
    fm->setPreviewFontPath("/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf");
    fm->onPreviewFontChanged();
    EXPECT_FALSE(fm->m_previewTexts.isEmpty());
}

TEST_F(TestDFontPreviewer, checkPaintevent)
{
    Stub st;
    st.set((void (QPainter::*)(const QRect &, int, const QString &, QRect * br))ADDR(QPainter, drawText), stub_drawText);
    QRect r;
    QPaintEvent *p = new QPaintEvent(r);
    fm->paintEvent(p);
    SAFE_DELETE_ELE(p)
    EXPECT_TRUE(g_funcname == QLatin1String("stub_drawText"));
}


TEST_F(TestDFontPreviewer, removeUserAddFonts)
{

    GetFontListWorker getFontList(GetFontListWorker::AllInSquence);
    g_funcname.clear();
    getFontList.removeUserAddFonts();
    EXPECT_TRUE(g_funcname.isEmpty());

    Stub st;
    st.set((bool(QFile::*)())ADDR(QFile, remove), stub_remove);
    st.set(ADDR(QDir, removeRecursively), stub_removeRecursively);
    st.set(ADDR(DSqliteUtil, getInstalledFontsPath), stub_getInstalledFontsPath);
    DFontPreviewListDataThread::instance()->m_allFontPathList = QStringList(QString("/usr/share/myfonts/X11/Type1/c0648bt_.pfb"));
    getFontList.removeUserAddFonts();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_remove"));
}
