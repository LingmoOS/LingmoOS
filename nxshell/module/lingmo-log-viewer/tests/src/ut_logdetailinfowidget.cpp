// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logdetailinfowidget.h"

#include <gtest/gtest.h>
#include <stub.h>

#include <DLabel>

#include <QDebug>
#include <QPaintEvent>
#include <QTextBrowser>

TEST(logDetailInfoWidget_Constructor_UT, logDetailInfoWidget_Constructor_UT)
{
    logDetailInfoWidget *p = new logDetailInfoWidget(nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(logDetailInfoWidget_cleanText_UT, logDetailInfoWidget_cleanText_UT)
{
    logDetailInfoWidget *p = new logDetailInfoWidget(nullptr);
    EXPECT_NE(p, nullptr);
    p->cleanText();
    EXPECT_EQ(p->m_dateTime->isHioceann(), true);
    EXPECT_EQ(p->m_userName->isHioceann(), true);
    EXPECT_EQ(p->m_userLabel->isHioceann(), true);
    EXPECT_EQ(p->m_pid->isHioceann(), true);
    EXPECT_EQ(p->m_pidLabel->isHioceann(), true);
    EXPECT_EQ(p->m_action->isHioceann(), true);
    EXPECT_EQ(p->m_actionLabel->isHioceann(), true);
    EXPECT_EQ(p->m_status->isHioceann(), true);
    EXPECT_EQ(p->m_statusLabel->isHioceann(), true);

    EXPECT_EQ(p->m_level->isHioceann(), true);
    EXPECT_EQ(p->m_daemonName->isHioceann(), true);
    EXPECT_EQ(p->m_textBrowser->toPlainText().isEmpty(), true);
    EXPECT_EQ(p->m_name->isHioceann(), true);
    EXPECT_EQ(p->m_nameLabel->isHioceann(), true);
    EXPECT_EQ(p->m_event->isHioceann(), true);
    EXPECT_EQ(p->m_eventLabel->isHioceann(), true);
    p->deleteLater();
}

TEST(logDetailInfoWidget_hideLine_UT, logDetailInfoWidget_hideLine_UT)
{
    logDetailInfoWidget *p = new logDetailInfoWidget(nullptr);
    EXPECT_NE(p, nullptr);
    p->hideLine(true);
    EXPECT_EQ(p->m_hline->isHioceann(), true);
    p->deleteLater();
}

TEST(logDetailInfoWidget_initUI_UT, logDetailInfoWidget_initUI_UT)
{
    logDetailInfoWidget *p = new logDetailInfoWidget(nullptr);
    EXPECT_NE(p, nullptr);
    p->initUI();
    p->deleteLater();
}

TEST(logDetailInfoWidget_setTextCustomSize_UT, logDetailInfoWidget_setTextCustomSize_UT)
{
    logDetailInfoWidget *p = new logDetailInfoWidget(nullptr);
    EXPECT_NE(p, nullptr);
    QWidget *w = new QWidget;
    p->setTextCustomSize(w);
    w->deleteLater();
    p->deleteLater();
}

TEST(logDetailInfoWidget_fillDetailInfo_UT, logDetailInfoWidget_fillDetailInfo_UT_001)
{
    logDetailInfoWidget *p = new logDetailInfoWidget(nullptr);
    EXPECT_NE(p, nullptr);
    p->fillDetailInfo("", "", "", "", QModelIndex(), "");
    p->deleteLater();
}

TEST(logDetailInfoWidget_fillDetailInfo_UT, logDetailInfoWidget_fillDetailInfo_UT_002)
{
    logDetailInfoWidget *p = new logDetailInfoWidget(nullptr);
    EXPECT_NE(p, nullptr);
    p->fillDetailInfo("aa", "aa", "aa", "aa", QModelIndex(), "aa", "aa", "aa", "aa", "aa");
    p->deleteLater();
}

TEST(logDetailInfoWidget_fillOOCDetailInfo_UT, logDetailInfoWidget_fillOOCDetailInfo_UT_001)
{
    logDetailInfoWidget *p = new logDetailInfoWidget(nullptr);
    EXPECT_NE(p, nullptr);
    p->fillOOCDetailInfo("test");
    p->deleteLater();
}

TEST(logDetailInfoWidget_slot_DetailInfo_UT, logDetailInfoWidget_slot_DetailInfo_UT)
{
    logDetailInfoWidget *p = new logDetailInfoWidget(nullptr);
    EXPECT_NE(p, nullptr);
    p->slot_DetailInfo(QModelIndex(), nullptr, "", 0);
    p->deleteLater();
}
