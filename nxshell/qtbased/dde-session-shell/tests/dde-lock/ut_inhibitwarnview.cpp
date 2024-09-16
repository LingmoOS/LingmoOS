// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inhibitwarnview.h"

#include <QTest>

#include <gtest/gtest.h>

class UT_InhibitorRow : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    InhibitorRow *m_inhibitorRow;
};

class UT_InhibitWarnView : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    InhibitWarnView *m_inhibitWarnView;
};

void UT_InhibitorRow::SetUp()
{
    m_inhibitorRow = new InhibitorRow("", "");
}

void UT_InhibitorRow::TearDown()
{
    delete m_inhibitorRow;
}

TEST_F(UT_InhibitorRow, BasicTest)
{
    m_inhibitorRow->show();
}

void UT_InhibitWarnView::SetUp()
{
    m_inhibitWarnView = new InhibitWarnView(SessionBaseModel::RequireNormal);
}

void UT_InhibitWarnView::TearDown()
{
    delete m_inhibitWarnView;
}

TEST_F(UT_InhibitWarnView, BasicTest)
{
    m_inhibitWarnView->setInhibitorList(QList<InhibitWarnView::InhibitorData>());
    m_inhibitWarnView->setInhibitConfirmMessage("test");
    m_inhibitWarnView->setAcceptReason("test");
    m_inhibitWarnView->setAcceptVisible(false);
    m_inhibitWarnView->toggleButtonState();
    m_inhibitWarnView->buttonClickHandle();
    m_inhibitWarnView->inhibitType();
    m_inhibitWarnView->focusNextPrevChild(false);
    m_inhibitWarnView->setCurrentButton(ButtonType::Cancel);
    QTest::keyPress(m_inhibitWarnView, Qt::Key_Return, Qt::KeyboardModifier::NoModifier);
}
