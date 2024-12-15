// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_jobtypecombobox.h"
//#include "../third-party_stub/stub.h"

//static QList<JobTypeInfo> jobtypecombobox_stub_JobTypeInfos1()
//{
//    QList<JobTypeInfo> infos;
//    infos.push_back(JobTypeInfo(1, "1", 0, "#000000"));
//    infos.push_back(JobTypeInfo(2));
//    infos.push_back(JobTypeInfo(3, "3", 2, "#000000"));

//    return infos;
//}

//static QList<JobTypeInfo> jobtypecombobox_stub_JobTypeInfos2()
//{
//    QList<JobTypeInfo> infos;
//    infos.push_back(JobTypeInfo(1, "1", 0, "#000000"));
//    infos.push_back(JobTypeInfo(2, "2", 1, "#000000"));
//    infos.push_back(JobTypeInfo(3, "3", 2, "#000000"));

//    return infos;
//}

//static int jobtypecombobox_stub_flase()
//{
//    return 0;
//}

//test_jobtypecombobox::test_jobtypecombobox()
//{

//}

//TEST_F(test_jobtypecombobox, setAlert_001)
//{
//    mWidget->setAlert(true);
//    EXPECT_TRUE(mWidget->m_control->isAlert());
//}

//TEST_F(test_jobtypecombobox, isAlert_001)
//{
//    mWidget->setAlert(true);
//    EXPECT_TRUE(mWidget->isAlert());
//}

//TEST_F(test_jobtypecombobox, isAlert_002)
//{
//    mWidget->setAlert(false);
//    EXPECT_FALSE(mWidget->isAlert());
//}

//TEST_F(test_jobtypecombobox, showAlertMessage_001)
//{
//    mWidget->showAlertMessage("");
//}

//TEST_F(test_jobtypecombobox, showAlertMessage_002)
//{
//    mWidget->showAlertMessage("", mWidget);
//}

//TEST_F(test_jobtypecombobox, setAlertMessageAlignment_001)
//{
//    mWidget->setAlertMessageAlignment(Qt::AlignLeft);
//    EXPECT_EQ(mWidget->m_control->messageAlignment(), Qt::AlignLeft);
//}

//TEST_F(test_jobtypecombobox, hideAlertMessage_001)
//{
//    mWidget->hideAlertMessage();
//}

//TEST_F(test_jobtypecombobox, getCurrentEditPositione_001)
//{
//    EXPECT_EQ(mWidget->getCurrentEditPosition(), mWidget->m_newPos);
//}

//TEST_F(test_jobtypecombobox, updateJobType_001)
//{
//    Stub stub;
//    stub.set(ADDR(JobTypeInfoManager, getJobTypeList), jobtypecombobox_stub_JobTypeInfos2);
//    EXPECT_TRUE(mWidget->updateJobType());
//}

//TEST_F(test_jobtypecombobox, getCurrentJobTypeNo_001)
//{
//    Stub stub;
//    stub.set(ADDR(JobTypeInfoManager, getJobTypeList), jobtypecombobox_stub_JobTypeInfos1);
//    stub.set(ADDR(QComboBox, currentIndex), jobtypecombobox_stub_flase);
//    mWidget->updateJobType();
//    mWidget->getCurrentJobTypeNo();
//}

//TEST_F(test_jobtypecombobox, setCurrentJobTypeNo_001)
//{
//    Stub stub;
//    stub.set(ADDR(JobTypeInfoManager, getJobTypeList), jobtypecombobox_stub_JobTypeInfos1);
//    mWidget->updateJobType();
//    mWidget->setCurrentJobTypeNo(3);
//}

//TEST_F(test_jobtypecombobox, slotBtnAddItemClicked_001)
//{
//    mWidget->slotBtnAddItemClicked();
//    EXPECT_TRUE(mWidget->isEditable());
//    EXPECT_TRUE(mWidget->currentText().isEmpty());
//}

//TEST_F(test_jobtypecombobox, showPopup_001)
//{
//    mWidget->showPopup();
//    EXPECT_FALSE(mWidget->isEditable());
//}

//TEST_F(test_jobtypecombobox, eventFilter_001)
//{
//    QPointF point(1, 1);
//    QEnterEvent e(point, point, point);
//    mWidget->showPopup();
//    mWidget->eventFilter(mWidget->m_addBtn, &e);
//}

//TEST_F(test_jobtypecombobox, eventFilter_002)
//{
//    Stub stub;
//    stub.set(ADDR(JobTypeInfoManager, getJobTypeList), jobtypecombobox_stub_JobTypeInfos2);
//    mWidget->updateJobType();
//    QPointF point(1, 1);
//    QFocusEvent e(QEvent::FocusIn);
//    mWidget->showPopup();
//    mWidget->eventFilter(mWidget->m_addBtn, &e);
//}

//TEST_F(test_jobtypecombobox, eventFilter_003)
//{
//    Stub stub;
//    stub.set(ADDR(JobTypeInfoManager, getJobTypeList), jobtypecombobox_stub_JobTypeInfos2);
//    mWidget->updateJobType();
//    QPointF point(1, 1);
//    QKeyEvent e(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
//    mWidget->showPopup();
//    mWidget->m_addBtn->setHighlight(true);
//    mWidget->eventFilter(mWidget->m_addBtn, &e);
//    EXPECT_TRUE(mWidget->m_addBtn->isHighlight());
//}

//TEST_F(test_jobtypecombobox, eventFilter_004)
//{
//    Stub stub;
//    stub.set(ADDR(JobTypeInfoManager, getJobTypeList), jobtypecombobox_stub_JobTypeInfos2);
//    mWidget->updateJobType();
//    QPointF point(1, 1);
//    QKeyEvent e(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
//    mWidget->showPopup();
//    mWidget->m_addBtn->setHighlight(true);
//    mWidget->eventFilter(mWidget->m_addBtn, &e);
//    EXPECT_TRUE(mWidget->m_addBtn->isHighlight());
//}

//TEST_F(test_jobtypecombobox, eventFilter_005)
//{
//    Stub stub;
//    stub.set(ADDR(JobTypeInfoManager, getJobTypeList), jobtypecombobox_stub_JobTypeInfos2);
//    mWidget->updateJobType();
//    QPointF point(1, 1);
//    QKeyEvent e(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
//    mWidget->showPopup();
//    mWidget->m_addBtn->setHighlight(true);
//    mWidget->eventFilter(mWidget->m_customWidget, &e);
//    EXPECT_TRUE(mWidget->m_addBtn->isHighlight());
//}

//TEST_F(test_jobtypecombobox, initUI_001)
//{
//    mWidget->initUI();
//    EXPECT_FALSE(mWidget->isEditable());
//}

//TEST_F(test_jobtypecombobox, addJobTypeItem_001)
//{
//    mWidget->addJobTypeItem(1, "#000000", "");
//}

//TEST_F(test_jobtypecombobox, addCustomWidget_001)
//{
//    QFrame *viewContainer = mWidget->findChild<QFrame *>();
//    mWidget->addCustomWidget(viewContainer);
//    EXPECT_TRUE(mWidget->m_addBtn != nullptr);
//}

//TEST_F(test_jobtypecombobox, setItemSelectable_001)
//{
//    Stub stub;
//    stub.set(ADDR(JobTypeInfoManager, getJobTypeList), jobtypecombobox_stub_JobTypeInfos2);
//    mWidget->updateJobType();
//    mWidget->showPopup();
//    mWidget->setItemSelectable(true);
//    EXPECT_FALSE(mWidget->m_addBtn->isHighlight());
//}

//TEST_F(test_jobtypecombobox, setItemSelectable_002)
//{
//    Stub stub;
//    stub.set(ADDR(JobTypeInfoManager, getJobTypeList), jobtypecombobox_stub_JobTypeInfos2);
//    mWidget->updateJobType();
//    mWidget->showPopup();
//    mWidget->setItemSelectable(false);
//}
