// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/dfquickinstallwindow.h"


#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "utils.h"
#include "commonheaderfile.h"

#include "views/dfinstallnormalwindow.h"
#include "dfontinfomanager.h"
#include "fontmanagercore.h"
#include "dfmdbmanager.h"
#include "utils.h"
#include "globaldef.h"
#include "interfaces/dfontpreviewer.h"

#include <DLog>
#include <DPalette>
#include <DPushButton>
#include <DTitlebar>
#include <DApplicationHelper>
#include <DWidgetUtil>

#include <QTest>
#include <QSignalSpy>
#include <QFontDatabase>
#include <QResizeEvent>
#include <QVBoxLayout>

namespace {
class TestDFQuickInstallWindow : public testing::Test
{

protected:
    void SetUp()
    {
        fqi = new DFQuickInstallWindow(QStringList(), w);
    }
    void TearDown()
    {
        delete fqi;
    }

    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFQuickInstallWindow *fqi;
};

//参数化测试
class TestcheckInitPreviewFont : public::testing::TestWithParam<QString>
{
public:
    QWidget *w = new QWidget;
    DFQuickInstallWindow *fqi = new DFQuickInstallWindow(QStringList(), w);
};

DFontInfo stub_getFontInfoFirst(const QString &filePath)
{
    Q_UNUSED(filePath)
    DFontInfo f;

    f.isError = false;
    f.isInstalled = true;

    f.familyName = "familyname";
    return f;
}

DFontInfo stub_getFontInfoSecond(const QString &filePath)
{
    Q_UNUSED(filePath)
    DFontInfo f;

    f.isError = false;
    f.isInstalled = false;

    return f;
}

DFontInfo stub_getFontInfoThird(const QString &filePath)
{
    Q_UNUSED(filePath)
    DFontInfo f;

    f.isError = false;
    f.isInstalled = false;
    f.styleName = "first";

    return f;
}



void stub_installFont(const QStringList &files)
{
    Q_UNUSED(files)
    return ;
}
static QString g_funcname;
void stub_resizeEvent(void *, QResizeEvent *)
{
    g_funcname = __FUNCTION__;
}
}

TEST_F(TestDFQuickInstallWindow, checkOnFileSelectedFirst)
{
    QStringList list;
    list << "aaaaaaaaaa";
    fqi->onFileSelected(list);
    EXPECT_TRUE(fqi->m_installFiles.count() == 1);
    EXPECT_TRUE(fqi->m_stateLabel->text() == QLatin1String("Broken file"));
    EXPECT_TRUE(fqi->m_fontType->itemText(0) == QLatin1String("Unknown"));

    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoFirst);
    fqi->onFileSelected(list);
    EXPECT_TRUE(fqi->m_stateLabel->text() == QLatin1String("Installed"));
    EXPECT_TRUE(fqi->m_fontType->itemText(0) == QLatin1String("Unknown"));
    EXPECT_TRUE(fqi->m_titleLabel->text() == QLatin1String("familyname"));
}

TEST_F(TestDFQuickInstallWindow, checkOnFileSelectedSecond)
{
    QStringList list;
    list << "aaaaaaaaaa";
    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoSecond);
    fqi->onFileSelected(list);
    EXPECT_TRUE(fqi->m_installFiles.count() == 1);
    EXPECT_TRUE(fqi->m_stateLabel->text() == QLatin1String("Not Installed"));
    EXPECT_TRUE(fqi->m_fontType->itemText(0) == QLatin1String("Unknown"));
}

TEST_F(TestDFQuickInstallWindow, checkresizeEvent)
{
    QSize s;
    QResizeEvent *e = new QResizeEvent(s, s);
    typedef void (*fptr)(DMainWindow *, QResizeEvent);
    fptr DMainWindow_resizeEvent = (fptr)(&DMainWindow::resizeEvent);   //获取虚函数地址
    Stub stub;
    stub.set(DMainWindow_resizeEvent, stub_resizeEvent);

    fqi->resizeEvent(e);
    SAFE_DELETE_ELE(e)
    EXPECT_TRUE(g_funcname == QLatin1String("stub_resizeEvent"));
}


TEST_F(TestDFQuickInstallWindow, checkOnInstallBtnClicked)
{
    QSignalSpy spy(fqi, SIGNAL(quickInstall()));

    Stub s;
    s.set(ADDR(DFQuickInstallWindow, installFont), stub_installFont);

    QTest::mouseClick(fqi->m_actionBtn, Qt::LeftButton);



    EXPECT_TRUE(spy.count() == 1);
}


TEST_F(TestDFQuickInstallWindow, checkOnFontInstallFinishedFirst)
{
    QSignalSpy spy(fqi, SIGNAL(requestShowMainWindow(const QStringList &)));

    fqi->m_installFiles << "first";

    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoSecond);

    fqi->onFontInstallFinished();

    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFQuickInstallWindow, checkOnFontInstallFinishedSecond)
{
    QSignalSpy spy(fqi, SIGNAL(requestShowMainWindow(const QStringList &)));

    fqi->m_installFiles << "second";

    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoThird);

    fqi->onFontInstallFinished();

    EXPECT_TRUE(spy.count() == 1);
}

TEST_P(TestcheckInitPreviewFont, checkInitPreviewFont)
{
    QString n =  GetParam();
    DFontInfo f;
    f.familyName = "familyName";
    f.styleName = QString(n);

    fqi->InitPreviewFont(f);
    EXPECT_TRUE(fqi->m_titleLabel->text() == "familyName");
}

INSTANTIATE_TEST_CASE_P(HandleTrueReturn, TestcheckInitPreviewFont, testing::Values("Italic", "Regular", "ExtraLight", "ExtraBold",
                                                                                    "DemiBold", "Bold", "Light", "Thin", "Medium", "Black"));



