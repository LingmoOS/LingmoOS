// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_jobtypelistview.h"
//#include "cscheduleoperation.h"
//#include "../dialog_stub.h"

//static int jobtypelistview_stub_true(int)
//{
//    return 1;
//}

//test_jobtypelistview::test_jobtypelistview()
//{

//}

//TEST_F(test_jobtypelistview, canAdd_001)
//{
//    mWidget->canAdd();
//}

//TEST_F(test_jobtypelistview, viewportEvent_001)
//{
//    QHoverEvent e(QEvent::HoverLeave, QPointF(0, 0), QPointF(0, 0));
//    mWidget->viewportEvent(&e);
//}

//TEST_F(test_jobtypelistview, viewportEvent_002)
//{
//    QHoverEvent e(QEvent::HoverLeave, QPointF(0, 0), QPointF(0, 0));
//    mWidget->m_iIndexCurrentHover = 0;
//    mWidget->viewportEvent(&e);
//}

//TEST_F(test_jobtypelistview, viewportEvent_003)
//{
//    QHoverEvent e(QEvent::HoverMove, QPointF(1, 1), QPointF(1, 1));
//    mWidget->m_iIndexCurrentHover = 1;
//    mWidget->viewportEvent(&e);
//}

//TEST_F(test_jobtypelistview, initUI_001)
//{
//    mWidget->initUI();
//}

//TEST_F(test_jobtypelistview, addJobTypeItem_001)
//{
//    mWidget->addJobTypeItem(JobTypeInfo());
//}

//TEST_F(test_jobtypelistview, slotUpdateJobType_001)
//{
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    mWidget->slotUpdateJobType();
//}

//TEST_F(test_jobtypelistview, slotDeleteJobType_001)
//{
//    mWidget->slotDeleteJobType();
//}

//TEST_F(test_jobtypelistview, slotDeleteJobType_002)
//{
//    mWidget->m_iIndexCurrentHover = 1;
//    mWidget->slotDeleteJobType();
//}

//TEST_F(test_jobtypelistview, slotDeleteJobType_003)
//{
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    stub.set(ADDR(CScheduleOperation, isJobTypeUsed), jobtypelistview_stub_true);
//    mWidget->m_iIndexCurrentHover = 1;
//    mWidget->slotDeleteJobType();
//}

//TEST_F(test_jobtypelistview, updateJobType_003)
//{
//    mWidget->m_iIndexCurrentHover = 1;
//    mWidget->updateJobType();
//    EXPECT_EQ(mWidget->m_iIndexCurrentHover, -1);
//}
