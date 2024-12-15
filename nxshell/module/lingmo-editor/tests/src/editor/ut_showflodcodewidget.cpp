// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_showflodcodewidget.h"
#include "../../src/editor/showflodcodewidget.h"

test_showflodcodewidget::test_showflodcodewidget()
{

}

TEST_F(test_showflodcodewidget, ShowFlodCodeWidget)
{
    ShowFlodCodeWidget flodCodeWidget(nullptr);
    ASSERT_TRUE(flodCodeWidget.m_pContentEdit != nullptr);
}

//void appendText(QString strText, int maxWidth);
TEST_F(test_showflodcodewidget, appendText)
{
    ShowFlodCodeWidget *flodCodeWidget = new ShowFlodCodeWidget();
    flodCodeWidget->appendText("aa",1);
    ASSERT_TRUE(flodCodeWidget->m_pContentEdit->textCursor().position() == 0);

    flodCodeWidget->deleteLater();
}

//void clear();
TEST_F(test_showflodcodewidget, clear)
{
    ShowFlodCodeWidget *flodCodeWidget = new ShowFlodCodeWidget();
    flodCodeWidget->clear();

    ASSERT_TRUE(flodCodeWidget->m_nTextWidth == 0);
    flodCodeWidget->deleteLater();
}

//void initHighLight(QString filepath, bool bIsLight);
TEST_F(test_showflodcodewidget, initHighLight)
{
    ShowFlodCodeWidget *flodCodeWidget = new ShowFlodCodeWidget();
    flodCodeWidget->initHighLight("aa",true);

    ASSERT_TRUE(flodCodeWidget->m_pContentEdit != nullptr);
    flodCodeWidget->deleteLater();
}

//void setStyle(bool bIsLineWrap);
TEST_F(test_showflodcodewidget, setStyle)
{
    ShowFlodCodeWidget *flodCodeWidget = new ShowFlodCodeWidget();
    flodCodeWidget->setStyle(true);

    ASSERT_TRUE(flodCodeWidget->m_pContentEdit->lineWrapMode() == QPlainTextEdit::WidgetWidth);
    flodCodeWidget->deleteLater();
}

//void hideFirstBlock();
TEST_F(test_showflodcodewidget, hideFirstBlock)
{
    ShowFlodCodeWidget *flodCodeWidget = new ShowFlodCodeWidget();
    flodCodeWidget->hideFirstBlock();

    ASSERT_TRUE(flodCodeWidget->m_pContentEdit->height() >= 10);
    flodCodeWidget->deleteLater();
}
