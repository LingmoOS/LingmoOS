// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "func.h"
#include "messagebox.h"
#include "settings.h"
#include "stub.h"
#include "stubAll.h"
#include <QObject>
#include <QPushButton>
class ut_messageBox : public ::testing::Test
{
protected:
    ut_messageBox()
    {
    }

    virtual ~ut_messageBox()
    {
    }
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_messageBox, setUnusual)
{
    MessageBox msg;
    msg.setUnusual("", "");
}

TEST_F(ut_messageBox, setExit)
{
    MessageBox msg;
    msg.setExit();
}

TEST_F(ut_messageBox, setFolderDenied)
{
    MessageBox msg;
    msg.setFolderDenied();
}

TEST_F(ut_messageBox, onClearBtnClicked)
{
    MessageBox msg;
    msg.m_CheckBox = new DCheckBox;
    msg.onClearBtnClicked(0);
    msg.onClearBtnClicked(1);
    delete msg.m_CheckBox;
}

TEST_F(ut_messageBox, onDeleteBtnClicked)
{
    MessageBox msg;
    msg.m_CheckBox = new DCheckBox;
    msg.onDeleteBtnClicked(0);
    msg.setDelete(true);
    msg.m_DeleteFlag = true;
    msg.onDeleteBtnClicked(1);
    msg.m_DeleteFlag = false;
    msg.onDeleteBtnClicked(1);
    delete msg.m_CheckBox;
}

TEST_F(ut_messageBox, onExitBtnClicked)
{
    MessageBox *msg = new MessageBox;
//    if(msg.m_ButtonMin != nullptr){
//        delete msg.m_ButtonMin;
//        msg.m_ButtonMin = nullptr;
//    }
//    if(msg.m_ButtonQuit != nullptr){
//        delete msg.m_ButtonQuit;
//        msg.m_ButtonQuit = nullptr;
//    }
    msg->m_ButtonMin = new DRadioButton(msg);
    msg->m_ButtonQuit = new DRadioButton(msg);
    msg->onExitBtnClicked(0);
    msg->addCheckbox("11");
    msg->addRadioGroup("11", "22");
    msg->m_CheckBox->setCheckState(Qt::CheckState::Checked);
    msg->m_ButtonMin->setChecked(true);
    msg->onExitBtnClicked(1);
    msg->m_CheckBox->setCheckState(Qt::CheckState::Unchecked);
    msg->onExitBtnClicked(1);
    msg->m_CheckBox->setCheckState(Qt::CheckState::Checked);
    msg->m_ButtonMin->setChecked(false);
    msg->onExitBtnClicked(1);
    msg->m_ButtonQuit->click();
    msg->m_ButtonMin->click();
    delete msg->m_ButtonMin;
    msg->m_ButtonMin = nullptr;
    delete msg->m_ButtonQuit;
    msg->m_ButtonQuit = nullptr;
    delete msg;
    msg = nullptr;
}

TEST_F(ut_messageBox, setWarings)
{
    MessageBox msg;
    msg.setWarings("warring", "warring");
}

TEST_F(ut_messageBox, setWarings1)
{
    MessageBox msg;
    QStringList list;
    list << "11" << "22";
    msg.setWarings("warring", "warring", "warring",2,list);
}

TEST_F(ut_messageBox, setRedownload)
{
    MessageBox msg;
    msg.setRedownload("");
    msg.setRedownload("11",true,true);
}

TEST_F(ut_messageBox, setDelete)
{
    MessageBox msg;
    msg.setDelete(true);
    msg.setDelete(false);
    msg.setDelete(true, true);
    msg.setDelete(false, true);
}
TEST_F(ut_messageBox, onRenamelineeditChanged)
{
    MessageBox msg;
    msg.m_RenameSureButton = new QPushButton();
    msg.onRenamelineeditChanged("text.ddd");
    msg.onRenamelineeditChanged("text");
    msg.onRenamelineeditChanged("");
    delete msg.m_RenameSureButton;
}

TEST_F(ut_messageBox, setClear)
{
    MessageBox msg;
    msg.setClear();
}

TEST_F(ut_messageBox, addCheckbox)
{
    MessageBox msg;
    msg.addCheckbox("11", true);
}

TEST_F(ut_messageBox, setNetWorkError)
{
    MessageBox msg;
    msg.setNetWorkError("11");
}

TEST_F(ut_messageBox, addRadioGroup)
{
    typedef int (*fptr)(MessageBox *);
    fptr foo = (fptr)(&MessageBox::exec);
    Stub stub1;
    stub1.set(foo, MessageboxExec);



    MessageBox msg;
    Settings::getInstance()->setCloseMainWindowSelected(1);
    msg.addRadioGroup("11","11");
    Settings::getInstance()->setCloseMainWindowSelected(0);
    msg.addRadioGroup("11","11");

    msg.m_NewnameLineedit = new DLineEdit;
    msg.m_NewnameLineedit->setText("ff\\ffff/ff");
    msg.onRenameSureBtnClicked();
    delete msg.m_NewnameLineedit;
}


