// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontwidget.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include <QFile>
#include <QTimer>
#include <QTranslator>
#include <QDebug>

namespace {
class TestDFontWidget : public testing::Test
{
protected:
    void SetUp()
    {
        fw = new DFontWidget();
    }
    void TearDown()
    {
        delete  fw;
    }
    // Some expensive resource shared by all tests.
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    DFontWidget *fw;
};

static QString g_funcname;
int stub_addApplicationFontFromData(const QByteArray &)
{
    return 0;
}
void stub_setFileUrl(const QString &)
{
    g_funcname = __FUNCTION__;
}
}

TEST_F(TestDFontWidget, initTest)
{

}

TEST_F(TestDFontWidget, checkSetFileUrl)
{
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    fw->setFileUrl(filepath);
    EXPECT_EQ(true, fw->m_spinner->isPlaying());
    EXPECT_EQ(true, fw->m_layout->currentIndex() == 0);
    EXPECT_EQ(true, fw->m_filePath == filepath);
    fw->m_thread->quit();
    fw->m_thread->wait();
}

TEST_F(TestDFontWidget, checkHandleFinished_dataEmpty)
{
    QByteArray data;
    fw->handleFinished(data);
    EXPECT_EQ(true, fw->m_layout->currentIndex() == 0);
    EXPECT_EQ(false, fw->m_preview->isVisible());
    EXPECT_TRUE(fw->m_errMsg->text() == DApplication::translate("DFontWidget", "Broken file"));
}

TEST_F(TestDFontWidget, checkHandleFinished_data)
{
    Stub s;
    s.set(ADDR(QFontDatabase, addApplicationFontFromData), stub_addApplicationFontFromData);
    s.set(ADDR(DFontPreview, setFileUrl), stub_setFileUrl);

    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    QByteArray fileContent = file.readAll();
    fw->handleFinished(fileContent);
    EXPECT_FALSE(fw->m_errMsg->text() == DApplication::translate("DFontWidget", "Broken file"));
    EXPECT_TRUE(g_funcname == QLatin1String("stub_setFileUrl"));
}
