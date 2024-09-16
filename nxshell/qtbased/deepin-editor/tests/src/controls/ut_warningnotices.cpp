// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_warningnotices.h"
#include "../../src/controls/warningnotices.h"

test_warningnotices::test_warningnotices()
{

}

//explicit WarningNotices(MessageType notifyType = MessageType::ResidentType);
TEST_F(test_warningnotices, WarningNotices)
{
    WarningNotices notices(WarningNotices::ResidentType);
    
}

//void setReloadBtn();
TEST_F(test_warningnotices, setReloadBtn)
{
    WarningNotices *notices = new WarningNotices(WarningNotices::ResidentType);
    notices->m_reloadBtn->setVisible(false);
    notices->setReloadBtn();

    EXPECT_EQ(notices->m_reloadBtn->isVisible(),false);

    notices->setReloadBtn();
    notices->slotreloadBtnClicked();
    EXPECT_EQ(notices->isVisible(),false);


    EXPECT_NE(notices,nullptr);
    notices->deleteLater();
    
}

//void setSaveAsBtn();
TEST_F(test_warningnotices, setSaveAsBtn)
{
    WarningNotices *notices = new WarningNotices(WarningNotices::ResidentType);
    notices->setSaveAsBtn();
    notices->slotsaveAsBtnClicked();


    EXPECT_EQ(notices->m_saveAsBtn->isVisible(),false);

    EXPECT_NE(notices,nullptr);
    notices->deleteLater();
    
}

