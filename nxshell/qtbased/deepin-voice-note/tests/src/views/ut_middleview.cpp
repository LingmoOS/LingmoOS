// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_middleview.h"
#include "middleview.h"
#include "middleviewdelegate.h"
#include "vnoteitem.h"
#include "middleviewsortfilter.h"
#include "vnotedatamanager.h"
#include "vnoteitemoper.h"
#include "vnoteforlder.h"
#include "stub.h"

#include <standarditemcommon.h>
#include <vnoteitemoper.h>

#include <DFileDialog>
DWIDGET_USE_NAMESPACE

static int stub_dialog()
{
    return 1;
}

UT_MiddleView::UT_MiddleView()
{
}

void UT_MiddleView::SetUp()
{
    m_middleView = new MiddleView;
    VNoteFolder *folder = VNoteDataManager::instance()->getNoteFolders()->folders[0];
    VNoteItemOper noteOper;
    VNOTE_ITEMS_MAP *notes = noteOper.getFolderNotes(folder->id);
    if (notes) {
        notes->lock.lockForRead();
        int i = 0;
        for (auto it : notes->folderNotes) {
            it->isTop = i % 2 == 0 ? true : false;
            i++;
            m_middleView->appendRow(it);
        }
        notes->lock.unlock();
    }
}

void UT_MiddleView::TearDown()
{
    delete m_middleView;
}

TEST_F(UT_MiddleView, setSearchKey)
{
    m_middleView->grab();
    m_middleView->setSearchKey("note");
    EXPECT_EQ(m_middleView->m_searchKey, QString("note"));
    m_middleView->grab();
    m_middleView->m_pItemDelegate->handleChangeTheme();
    m_middleView->clearAll();
    EXPECT_FALSE(m_middleView->rowCount());
    m_middleView->closeMenu();
    EXPECT_FALSE(m_middleView->m_noteMenu->isVisible());
}

TEST_F(UT_MiddleView, setCurrentId)
{
    MiddleView middleview;
    middleview.setCurrentId(1);
    EXPECT_EQ(1, middleview.getCurrentId());
}

TEST_F(UT_MiddleView, UT_MiddleView_mousePressEvent_001)
{
    m_middleView->selectAll();
    QModelIndex index = m_middleView->currentIndex();
    QPoint localPos, globalPos;
    if (index.isValid()) {
        QRect rc = m_middleView->visualRect(index);
        localPos = rc.center();
        globalPos = m_middleView->mapToGlobal(localPos);
    }
    m_middleView->m_onlyCurItemMenuEnable = false;

    QMouseEvent *mousePressEvent = new QMouseEvent(QEvent::MouseButtonPress, localPos, localPos, localPos, Qt::LeftButton, Qt::RightButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    m_middleView->mousePressEvent(mousePressEvent);

    mousePressEvent->setModifiers(Qt::ControlModifier);
    m_middleView->mousePressEvent(mousePressEvent);

    m_middleView->setModifierState(MiddleView::shiftAndMouseModifier);
    m_middleView->mousePressEvent(mousePressEvent);

    mousePressEvent->setModifiers(Qt::ShiftModifier);
    m_middleView->mousePressEvent(mousePressEvent);

    m_middleView->m_onlyCurItemMenuEnable = true;
    m_middleView->mousePressEvent(mousePressEvent);
    delete mousePressEvent;

    m_middleView->m_onlyCurItemMenuEnable = false;
    QMouseEvent *rMousePressEvent = new QMouseEvent(QEvent::MouseButtonPress, localPos, localPos, localPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    m_middleView->mousePressEvent(rMousePressEvent);

    rMousePressEvent->setModifiers(Qt::ShiftModifier);
    m_middleView->mousePressEvent(rMousePressEvent);

    rMousePressEvent->setModifiers(Qt::ControlModifier);
    m_middleView->mousePressEvent(rMousePressEvent);
    delete rMousePressEvent;

    //doubleClickEvent
    m_middleView->m_onlyCurItemMenuEnable = false;
    QMouseEvent *doubleClickEvent = new QMouseEvent(QEvent::MouseButtonDblClick, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_middleView->mouseDoubleClickEvent(doubleClickEvent);
    delete doubleClickEvent;

    //mouseMoveEvent
    m_middleView->m_onlyCurItemMenuEnable = false;
    QMouseEvent *mouseMoveEvent = new QMouseEvent(QEvent::MouseMove, localPos, localPos, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    m_middleView->setTouchState(MiddleView::TouchMoving);
    m_middleView->mouseMoveEvent(mouseMoveEvent);
    delete mouseMoveEvent;
}

TEST_F(UT_MiddleView, UT_MiddleView_mousePressEvent_002)
{
    QModelIndex index = m_middleView->currentIndex();
    QPoint localPos, globalPos;
    if (index.isValid()) {
        QRect rc = m_middleView->visualRect(index);
        localPos = rc.center();
        globalPos = m_middleView->mapToGlobal(localPos);
    }
    m_middleView->m_onlyCurItemMenuEnable = false;

    QMouseEvent *mousePressEvent = new QMouseEvent(QEvent::MouseButtonPress, localPos, localPos, localPos, Qt::LeftButton, Qt::RightButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);

    mousePressEvent->setModifiers(Qt::ControlModifier);
    m_middleView->mousePressEvent(mousePressEvent);
    delete mousePressEvent;

    QMouseEvent *mousePressEvent2 = new QMouseEvent(QEvent::MouseButtonPress, QPoint(0, 0), QPoint(0, 0), QPoint(0, 0),
                                                    Qt::RightButton, Qt::RightButton, Qt::NoModifier,
                                                    Qt::MouseEventSource::MouseEventNotSynthesized);
    m_middleView->m_menuState = MiddleView::MenuStatus::ReleaseFromMenu;
    m_middleView->mousePressEvent(mousePressEvent2);
    delete mousePressEvent2;
}

TEST_F(UT_MiddleView, UT_MiddleView_mouseReleaseEvent_001)
{
    m_middleView->selectAll();
    QModelIndex index = m_middleView->currentIndex();
    QPoint localPos, globalPos;
    if (index.isValid()) {
        QRect rc = m_middleView->visualRect(index);
        localPos = rc.center();
        globalPos = m_middleView->mapToGlobal(localPos);
    }

    QMouseEvent *releaseEvent = new QMouseEvent(QEvent::MouseButtonRelease, localPos,
                                                Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    m_middleView->m_onlyCurItemMenuEnable = true;
    m_middleView->mouseReleaseEvent(releaseEvent);

    m_middleView->m_onlyCurItemMenuEnable = false;
    m_middleView->mouseReleaseEvent(releaseEvent);

    m_middleView->m_touchState = MiddleView::TouchState::TouchDraging;
    m_middleView->mouseReleaseEvent(releaseEvent);
    delete releaseEvent;

    releaseEvent = new QMouseEvent(QEvent::MouseButtonRelease, localPos,
                                   Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

    m_middleView->m_touchState = MiddleView::TouchState::TouchPressing;
    releaseEvent->setModifiers(Qt::NoModifier);
    m_middleView->mouseReleaseEvent(releaseEvent);
    delete releaseEvent;
}

TEST_F(UT_MiddleView, setTouchState)
{
    MiddleView middleview;
    middleview.setTouchState(middleview.TouchNormal);
    EXPECT_EQ(m_middleView->m_touchState, middleview.TouchNormal);
}

TEST_F(UT_MiddleView, doTouchMoveEvent)
{
    MiddleView middleview;
    QPointF localPos;
    QMouseEvent *mouseMoveEvent = new QMouseEvent(QEvent::MouseMove, localPos, localPos, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    middleview.m_touchPressPoint = QPoint(localPos.x(), localPos.y() - 11);
    QDateTime time = QDateTime::currentDateTime();
    middleview.m_touchPressStartMs = time.toMSecsSinceEpoch() - 260;
    middleview.setTouchState(MiddleView::TouchMoving);
    EXPECT_EQ(middleview.m_touchState, MiddleView::TouchMoving);
    middleview.doTouchMoveEvent(mouseMoveEvent);
    delete mouseMoveEvent;
}

TEST_F(UT_MiddleView, setOnlyCurItemMenuEnable)
{
    MiddleView middleview;
    bool enable = false;
    middleview.setOnlyCurItemMenuEnable(enable);
    EXPECT_EQ(middleview.m_onlyCurItemMenuEnable, enable);
}

TEST_F(UT_MiddleView, eventFilter)
{
    MiddleView middleview;
    QFocusEvent focusInEvent(QEvent::FocusIn, Qt::TabFocusReason);
    middleview.eventFilter(&middleview, &focusInEvent);
    EXPECT_EQ(middleview.m_pItemDelegate->m_tabFocus, true);
    QFocusEvent focusOutEvent(QEvent::FocusOut);
    middleview.eventFilter(&middleview, &focusOutEvent);
    EXPECT_EQ(middleview.m_pItemDelegate->m_tabFocus, false);
    QKeyEvent *event1 = new QKeyEvent(QEvent::Destroy, 0x01000001, Qt::NoModifier, "test");
    EXPECT_FALSE(middleview.eventFilter(nullptr, event1));
    middleview.eventFilter(nullptr, &focusInEvent);
    EXPECT_EQ(middleview.m_pItemDelegate->m_editVisible, true);
    delete event1;
}

TEST_F(UT_MiddleView, addRowAtHead)
{
    MiddleView middleview;
    VNoteItem *vnoteitem = new VNoteItem;
    vnoteitem->noteId = 2;
    vnoteitem->folderId = 2;
    vnoteitem->noteTitle = "test";
    middleview.addRowAtHead(vnoteitem);
    EXPECT_EQ(1, middleview.rowCount());
    delete vnoteitem;
}

TEST_F(UT_MiddleView, appendRow)
{
    MiddleView middleview;
    VNoteItem *noteData = new VNoteItem;
    VNoteItem *noteData1 = new VNoteItem;
    middleview.appendRow(noteData);
    middleview.appendRow(noteData1);
    EXPECT_EQ(2, middleview.rowCount());
    middleview.sortView();
    noteData1->isTop = 1;
    middleview.sortView();
    middleview.m_pSortViewFilter->sortView(MiddleViewSortFilter::title);
    EXPECT_EQ(middleview.m_pSortViewFilter->m_sortFeild, MiddleViewSortFilter::title);
    middleview.m_pSortViewFilter->sortView(MiddleViewSortFilter::createTime);
    EXPECT_EQ(middleview.m_pSortViewFilter->m_sortFeild, MiddleViewSortFilter::createTime);
    delete noteData;
    delete noteData1;
}

TEST_F(UT_MiddleView, rowCount)
{
    MiddleView middleview;
    EXPECT_EQ(qint32(0), middleview.rowCount());
}

TEST_F(UT_MiddleView, editNote)
{
    m_middleView->setCurrentIndex(0);
    m_middleView->editNote();
}

TEST_F(UT_MiddleView, UT_MiddleView_saveAs_001)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&DFileDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_dialog);
    EXPECT_TRUE(m_middleView->selectedIndexes().isEmpty());
    m_middleView->saveAs(MiddleView::Note);
    m_middleView->saveAs(MiddleView::Text);
    m_middleView->saveAs(MiddleView::Voice);
    m_middleView->saveAs(MiddleView::Html);
}

TEST_F(UT_MiddleView, UT_MiddleView_saveAs_002)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&DFileDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_dialog);
    stub.set(ADDR(QModelIndexList, size), stub_dialog);
    m_middleView->selectAll();
    EXPECT_FALSE(m_middleView->selectedIndexes().isEmpty());
    m_middleView->saveAs(MiddleView::Note);
    m_middleView->saveAs(MiddleView::Text);
    m_middleView->saveAs(MiddleView::Voice);
    m_middleView->saveAs(MiddleView::Html);
}

TEST_F(UT_MiddleView, saveRecords)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&DFileDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_dialog);
    m_middleView->selectAll();
    EXPECT_FALSE(m_middleView->selectedIndexes().isEmpty());
    m_middleView->saveAs(MiddleView::SaveAsType::Voice);
}

TEST_F(UT_MiddleView, handleShiftAndPress)
{
    m_middleView->setCurrentIndex(0);
    QModelIndex index = m_middleView->currentIndex();
    m_middleView->handleShiftAndPress(index);
    EXPECT_EQ(m_middleView->getModifierState(), MiddleView::shiftAndMouseModifier);
}

TEST_F(UT_MiddleView, keyPressEvent)
{
    QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::ShiftModifier, "test");
    m_middleView->initPositionStatus(0);
    EXPECT_EQ(m_middleView->m_shiftSelection, -1);
    EXPECT_EQ(m_middleView->m_currentRow, 0);

    m_middleView->keyPressEvent(event);
    m_middleView->initPositionStatus(1);
    EXPECT_EQ(m_middleView->m_currentRow, 1);
    m_middleView->keyPressEvent(event);
    m_middleView->initPositionStatus(0);
    EXPECT_EQ(m_middleView->m_currentRow, 0);
    m_middleView->m_shiftSelection = 1;
    m_middleView->keyPressEvent(event);
    event->setModifiers(Qt::NoModifier);
    m_middleView->keyPressEvent(event);

    QKeyEvent *event1 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::ShiftModifier, "test2");
    m_middleView->initPositionStatus(0);
    m_middleView->keyPressEvent(event1);
    m_middleView->initPositionStatus(1);
    EXPECT_EQ(m_middleView->m_currentRow, 1);
    m_middleView->m_shiftSelection = 0;
    m_middleView->keyPressEvent(event1);
    m_middleView->m_shiftSelection = -1;
    m_middleView->keyPressEvent(event1);

    QKeyEvent *event2 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Home, Qt::ShiftModifier, "test3");
    m_middleView->keyPressEvent(event2);
    event2->setModifiers(Qt::NoModifier);
    m_middleView->keyPressEvent(event2);

    QKeyEvent *event3 = new QKeyEvent(QEvent::KeyPress, Qt::Key_End, Qt::ShiftModifier, "test3");
    m_middleView->keyPressEvent(event3);
    event3->setModifiers(Qt::NoModifier);
    m_middleView->keyPressEvent(event3);

    QKeyEvent *event4 = new QKeyEvent(QEvent::KeyPress, Qt::Key_A, Qt::ControlModifier, "test3");
    m_middleView->keyPressEvent(event4);

    delete event;
    delete event1;
    delete event2;
    delete event3;
    delete event4;
}

TEST_F(UT_MiddleView, setVisibleEmptySearch)
{
    MiddleView middleview;
    middleview.setVisibleEmptySearch(true);
    EXPECT_FALSE(nullptr == middleview.m_emptySearch);
    middleview.setOnlyCurItemMenuEnable(true);
    EXPECT_EQ(middleview.m_onlyCurItemMenuEnable, true);
}

TEST_F(UT_MiddleView, setDragSuccess)
{
    MiddleView middleview;
    bool value = true;
    middleview.setDragSuccess(value);
    EXPECT_EQ(value, middleview.m_dragSuccess);
}

TEST_F(UT_MiddleView, changeRightView)
{
    MiddleView middleview;
    bool value = true;
    middleview.changeRightView(value);
}

TEST_F(UT_MiddleView, handleDragEvent)
{
    MiddleView middleview;
    middleview.m_onlyCurItemMenuEnable = false;
    bool value = true;
    middleview.handleDragEvent(value);
    EXPECT_EQ(middleview.m_touchState, MiddleView::TouchDraging);
}

TEST_F(UT_MiddleView, deleteModelIndexs)
{
    m_middleView->selectAll();
    QModelIndexList indexList = m_middleView->selectedIndexes();
    m_middleView->deleteModelIndexs(indexList);
    EXPECT_FALSE(m_middleView->rowCount());
}

TEST_F(UT_MiddleView, keyReleaseEvent)
{
    MiddleView middleview;
    middleview.m_shiftSelection = 1;
    QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, "test");
    middleview.keyReleaseEvent(event);
    EXPECT_EQ(middleview.m_shiftSelection, -1);
    middleview.m_shiftSelection = 1;
    QKeyEvent *event2 = new QKeyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "test");
    middleview.keyReleaseEvent(event2);
    EXPECT_EQ(middleview.m_shiftSelection, 1);
    delete event;
    delete event2;
}

TEST_F(UT_MiddleView, getSelectedCount)
{
    MiddleView middleview;
    EXPECT_EQ(middleview.getSelectedCount(), 0);
}

//TEST_F(ut_middleview_test,onMenuShow){
//    QPoint point(128,128);
//    MiddleView middleview;
//    middleview.selectionModel()->select(middleview.m_pSortViewFilter->index(0, 0), QItemSelectionModel::Select);
//    middleview.onMenuShow(point);
//    middleview.selectionModel()->select(middleview.m_pSortViewFilter->index(1, 0), QItemSelectionModel::Select);
//    middleview.onMenuShow(point);
//}

TEST_F(UT_MiddleView, haveText)
{
    m_middleView->selectAll();
    EXPECT_TRUE(m_middleView->getSelectedCount());
    EXPECT_TRUE(m_middleView->haveText());
}

TEST_F(UT_MiddleView, haveVoice)
{
    m_middleView->selectAll();
    EXPECT_TRUE(m_middleView->getSelectedCount());
    EXPECT_TRUE(m_middleView->haveVoice());
}

TEST_F(UT_MiddleView, handleTouchSlideEvent)
{
    MiddleView middleview;
    middleview.handleTouchSlideEvent(24, 11, QCursor::pos());
    EXPECT_EQ(middleview.m_touchPressPoint, QCursor::pos());
}

TEST_F(UT_MiddleView, isMultipleSelected)
{
    MiddleView middleview;
    EXPECT_FALSE(middleview.isMultipleSelected());
}

TEST_F(UT_MiddleView, setMouseState)
{
    MiddleView middleview;

    MiddleView::MouseState mouseState {MiddleView::MouseState::normal};
    middleview.setMouseState(mouseState);
    EXPECT_EQ(middleview.m_mouseState, mouseState);
}

TEST_F(UT_MiddleView, getModifierState)
{
    MiddleView middleview;
    EXPECT_EQ(MiddleView::noModifier, middleview.getModifierState());
}

TEST_F(UT_MiddleView, setModifierState)
{
    MiddleView middleview;
    MiddleView::ModifierState modifierState {MiddleView::ModifierState::noModifier};
    middleview.setModifierState(modifierState);
    EXPECT_EQ(modifierState, middleview.getModifierState());
}

TEST_F(UT_MiddleView, onNoteChanged)
{
    MiddleView middleview;
    middleview.onNoteChanged();
    EXPECT_EQ(middleview.m_shiftSelection, -1);
}

TEST_F(UT_MiddleView, getCurrVNotedataList)
{
    m_middleView->selectAll();
    QList<VNoteItem *> notes = m_middleView->getCurrVNotedataList();
    EXPECT_FALSE(notes.isEmpty());
}

TEST_F(UT_MiddleView, getCurrVNotedata)
{
    m_middleView->setCurrentIndex(0);
    EXPECT_FALSE(m_middleView->getCurrVNotedata() == nullptr);
}

TEST_F(UT_MiddleView, deleteCurrentRow)
{
    int count = m_middleView->rowCount();
    m_middleView->setCurrentIndex(0);
    m_middleView->deleteCurrentRow();
    EXPECT_EQ(m_middleView->rowCount(), count - 1);
}

TEST_F(UT_MiddleView, UT_MiddleView_onExportFinished_001)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_dialog);
    m_middleView->onExportFinished(3);
    m_middleView->onExportFinished(4);
}
