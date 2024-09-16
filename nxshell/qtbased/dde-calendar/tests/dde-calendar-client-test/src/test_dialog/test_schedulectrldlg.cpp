// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_schedulectrldlg.h"

#include <QEvent>
#include <QApplication>

test_schedulectrldlg::test_schedulectrldlg()
{
    mScheduleCtrlDlg = new CScheduleCtrlDlg();
}

test_schedulectrldlg::~test_schedulectrldlg()
{
    delete mScheduleCtrlDlg;
    mScheduleCtrlDlg = nullptr;
}

//void CScheduleCtrlDlg::setTheMe(const int type)
TEST_F(test_schedulectrldlg, setTheMe)
{
    mScheduleCtrlDlg->setTheMe(1);
    mScheduleCtrlDlg->setTheMe(2);
}

//void CScheduleCtrlDlg::buttonJudge(int id)
TEST_F(test_schedulectrldlg, buttonJudge)
{
    mScheduleCtrlDlg->buttonJudge(1);
}

//QAbstractButton *CScheduleCtrlDlg::addPushButton(QString btName, bool type)
TEST_F(test_schedulectrldlg, addPushButton)
{
    mScheduleCtrlDlg->addPushButton("schedule", true);
    mScheduleCtrlDlg->addPushButton("schedule", false);
}

//QAbstractButton *CScheduleCtrlDlg::addsuggestButton(QString btName, bool type)
TEST_F(test_schedulectrldlg, addsuggestButton)
{
    mScheduleCtrlDlg->addsuggestButton("schedule", true);
    mScheduleCtrlDlg->addsuggestButton("schedule", false);
}

//QAbstractButton *CScheduleCtrlDlg::addWaringButton(QString btName, bool type)
TEST_F(test_schedulectrldlg, addWaringButton)
{
    mScheduleCtrlDlg->addsuggestButton("schedule", true);
    mScheduleCtrlDlg->addsuggestButton("schedule", false);
}

//void CScheduleCtrlDlg::setText(QString str)
TEST_F(test_schedulectrldlg, setText)
{
    mScheduleCtrlDlg->setText("schedule");
}

//void CScheduleCtrlDlg::setInformativeText(QString str)
TEST_F(test_schedulectrldlg, setInfomativeText)
{
    mScheduleCtrlDlg->setInformativeText("schedule");
}

//int CScheduleCtrlDlg::clickButton()
TEST_F(test_schedulectrldlg, clickButton)
{
    mScheduleCtrlDlg->clickButton();
}

//changeEvent
TEST_F(test_schedulectrldlg, changeEvent)
{
    QEvent event(QEvent::FontChange);
    mScheduleCtrlDlg->setText(tr("You are changing the repeating rule of this event."));
    mScheduleCtrlDlg->setInformativeText(tr("Do you want to change all occurrences?"));
    mScheduleCtrlDlg->addPushButton(tr("Cancel", "button"), true);
    mScheduleCtrlDlg->addWaringButton(tr("Change All"), true);
    QApplication::sendEvent(mScheduleCtrlDlg, &event);
}
