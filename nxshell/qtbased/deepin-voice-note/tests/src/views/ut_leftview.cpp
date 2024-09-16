// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_leftview.h"
#include "leftview.h"
#include "leftviewdelegate.h"
#include "leftviewsortfilter.h"
#include "vnoteforlder.h"
#include "middleview.h"
#include "vnotedatamanager.h"
#include "globaldef.h"

#include <stub.h>
#include <QLineEdit>
#include <QDateTime>
#include <QMimeData>

static int stub_secs()
{
    return 500;
}

UT_LeftView::UT_LeftView()
{
}

TEST_F(UT_LeftView, UT_LeftView_getNotepadRoot_001)
{
    LeftView leftview;
    leftview.m_pItemDelegate->handleChangeTheme();
    EXPECT_TRUE(nullptr != leftview.getNotepadRoot());
}

TEST_F(UT_LeftView, UT_LeftView_getNotepadRootIndex)
{
    LeftView leftview;
    leftview.getNotepadRootIndex();
    EXPECT_TRUE(leftview.getNotepadRootIndex().isValid());
}

TEST_F(UT_LeftView, UT_LeftView_mouseEvent_001)
{
    LeftView leftview;
    QPointF localPos;
    //mousePressEvent
    leftview.m_onlyCurItemMenuEnable = false;
    QMouseEvent *mousePressEvent = new QMouseEvent(QEvent::MouseButtonPress, localPos, localPos, localPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    QMouseEvent *mousePressEvent2 = new QMouseEvent(QEvent::MouseButtonPress, localPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    leftview.mousePressEvent(mousePressEvent);

    leftview.mousePressEvent(mousePressEvent2);
    leftview.m_onlyCurItemMenuEnable = true;
    leftview.mousePressEvent(mousePressEvent);
    leftview.mousePressEvent(mousePressEvent2);
    delete mousePressEvent;
    delete mousePressEvent2;

    //doubleClickEvent
    leftview.m_onlyCurItemMenuEnable = false;
    QMouseEvent *doubleClickEvent = new QMouseEvent(QEvent::MouseButtonDblClick, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    leftview.mouseDoubleClickEvent(doubleClickEvent);
    delete doubleClickEvent;

    //mouseMoveEvent
    leftview.m_onlyCurItemMenuEnable = false;
    QMouseEvent *mouseMoveEvent = new QMouseEvent(QEvent::MouseMove, localPos, localPos, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    leftview.setTouchState(leftview.TouchMoving);
    leftview.mouseMoveEvent(mouseMoveEvent);
    delete mouseMoveEvent;

    QMouseEvent *mouseMoveEvent2 = new QMouseEvent(QEvent::MouseMove, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    leftview.setTouchState(leftview.TouchPressing);
    leftview.mouseMoveEvent(mouseMoveEvent2);
    delete mouseMoveEvent2;
}

TEST_F(UT_LeftView, UT_LeftView_mouseReleaseEvent_001)
{
    LeftView leftview;
    QPointF localPos(1000, 800);

    QMouseEvent *releaseEvent = new QMouseEvent(QEvent::MouseButtonRelease, localPos,
                                                Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    leftview.m_onlyCurItemMenuEnable = false;
    leftview.mouseReleaseEvent(releaseEvent);
    EXPECT_FALSE(leftview.m_isDraging);

    leftview.m_onlyCurItemMenuEnable = true;
    leftview.mouseReleaseEvent(releaseEvent);
    EXPECT_FALSE(leftview.m_isDraging);
    delete releaseEvent;
}

TEST_F(UT_LeftView, UT_LeftView_mouseReleaseEvent_002)
{
    LeftView leftview;
    QPointF localPos;

    QMouseEvent *releaseEvent = new QMouseEvent(QEvent::MouseButtonRelease, localPos,
                                                Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    leftview.m_touchState = LeftView::TouchState::TouchDraging;
    leftview.mouseReleaseEvent(releaseEvent);
    EXPECT_FALSE(leftview.m_isDraging);
    delete releaseEvent;
}

TEST_F(UT_LeftView, UT_LeftView_setTouchState_001)
{
    LeftView leftview;
    leftview.setTouchState(leftview.TouchNormal);
    EXPECT_EQ(LeftView::TouchNormal, leftview.m_touchState);
}

TEST_F(UT_LeftView, UT_LeftView_doTouchMoveEvent_001)
{
    Stub stub;
    stub.set(ADDR(QDateTime, toMSecsSinceEpoch), stub_secs);

    LeftView leftview;
    QPointF localPos(100, 100);
    QMouseEvent *mouseMoveEvent = new QMouseEvent(QEvent::MouseMove, localPos, localPos, localPos, Qt::LeftButton,
                                                  Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    leftview.m_touchState = LeftView::TouchPressing;
    leftview.m_isDraging = false;
    leftview.m_touchPressStartMs = 0;
    leftview.m_touchPressPoint = QPoint(localPos.x(), localPos.y() - 11);
    ;

    leftview.doTouchMoveEvent(mouseMoveEvent);
    delete mouseMoveEvent;
}

TEST_F(UT_LeftView, UT_LeftView_doTouchMoveEvent_002)
{
    Stub stub;
    stub.set(ADDR(QDateTime, toMSecsSinceEpoch), stub_secs);

    LeftView leftview;
    QPointF localPos(100, 100);
    QMouseEvent *mouseMoveEvent = new QMouseEvent(QEvent::MouseMove, localPos, localPos, localPos, Qt::LeftButton,
                                                  Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    leftview.m_touchState = LeftView::TouchPressing;
    leftview.m_isDraging = false;
    leftview.m_touchPressStartMs = 500;
    leftview.m_touchPressPoint = QPoint(localPos.x(), localPos.y() - 11);
    ;

    leftview.doTouchMoveEvent(mouseMoveEvent);
    EXPECT_EQ(LeftView::TouchMoving, leftview.m_touchState);
    delete mouseMoveEvent;
}

TEST_F(UT_LeftView, UT_LeftView_doTouchMoveEvent_003)
{
    Stub stub;
    stub.set(ADDR(QDateTime, toMSecsSinceEpoch), stub_secs);

    LeftView leftview;
    QPointF localPos(100, 100);
    QMouseEvent *mouseMoveEvent = new QMouseEvent(QEvent::MouseMove, localPos, localPos, localPos, Qt::LeftButton,
                                                  Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSource::MouseEventSynthesizedByQt);
    leftview.m_touchState = LeftView::TouchDraging;
    leftview.m_isDraging = false;
    leftview.doTouchMoveEvent(mouseMoveEvent);

    leftview.m_touchState = LeftView::TouchMoving;
    leftview.doTouchMoveEvent(mouseMoveEvent);
    EXPECT_EQ(LeftView::TouchOutVisibleRegion, leftview.m_touchState);

    leftview.m_touchState = LeftView::TouchNormal;
    leftview.doTouchMoveEvent(mouseMoveEvent);
    EXPECT_EQ(LeftView::TouchNormal, leftview.m_touchState);
    delete mouseMoveEvent;
}

TEST_F(UT_LeftView, UT_LeftView_handleDragEvent_001)
{
    LeftView leftview;
    bool isTouch = true;
    leftview.handleDragEvent(isTouch);
    EXPECT_EQ(LeftView::TouchDraging, leftview.m_touchState);
}

TEST_F(UT_LeftView, UT_LeftView_dropEvent_001)
{
    LeftView leftview;
    QPointF localPos;
    QMimeData *mimeData = new QMimeData;
    mimeData->setData(NOTES_DRAG_KEY, QByteArray());
    QDropEvent *event = new QDropEvent(localPos, Qt::MoveAction, mimeData,
                                       Qt::LeftButton, Qt::NoModifier, QEvent::Drop);
    leftview.dropEvent(event);
    delete event;
    delete mimeData;
}

TEST_F(UT_LeftView, UT_LeftView_setOnlyCurItemMenuEnable_001)
{
    LeftView leftview;
    bool enable = false;
    leftview.setOnlyCurItemMenuEnable(enable);
    EXPECT_EQ(enable, leftview.m_onlyCurItemMenuEnable);
}

TEST_F(UT_LeftView, UT_LeftView_keyPressEvent_001)
{
    LeftView leftview;
    leftview.m_onlyCurItemMenuEnable = true;
    QKeyEvent *event1 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier, "test");
    leftview.keyPressEvent(event1);
    leftview.m_onlyCurItemMenuEnable = false;
    QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Home, Qt::NoModifier, "test2");
    leftview.keyPressEvent(event);
    delete event;
    delete event1;
}

TEST_F(UT_LeftView, UT_LeftView_restoreNotepadItem_001)
{
    LeftView leftview;
    EXPECT_TRUE(!leftview.restoreNotepadItem().isValid());
}

TEST_F(UT_LeftView, UT_LeftView_addFolder_001)
{
    LeftView leftview;
    QModelIndex notepadRootIndex = leftview.getNotepadRootIndex();
    leftview.expand(notepadRootIndex);
    VNOTE_FOLDERS_MAP *folders = VNoteDataManager::instance()->getNoteFolders();
    for (auto it : folders->folders) {
        leftview.addFolder(it);
    }
    leftview.editFolder();
    EXPECT_TRUE(0 != leftview.folderCount());
    leftview.sort();
    int sortNum = 0;
    for (auto it : folders->folders) {
        it->sortNumber = sortNum++;
    }
    leftview.sort();
    EXPECT_FALSE(leftview.grab().isNull());
}

TEST_F(UT_LeftView, UT_LeftView_appendFolder_001)
{
    LeftView leftview;

    VNoteFolder folder;
    folder.notesCount = 1;
    folder.defaultIcon = 1;
    folder.name = "test";
    folder.iconPath = "test1";
    folder.createTime = QDateTime::currentDateTime();
    leftview.appendFolder(&folder);
    leftview.setDefaultNotepadItem();
    EXPECT_TRUE(leftview.currentIndex().isValid());
    VNoteFolder *removeFolder = leftview.removeFolder();
    EXPECT_EQ(&folder, removeFolder);
}

TEST_F(UT_LeftView, UT_LeftView_closeMenu_001)
{
    LeftView leftview;
    leftview.m_notepadMenu->setVisible(true);
    leftview.closeMenu();
    EXPECT_TRUE(!leftview.m_notepadMenu->isVisible());
}

TEST_F(UT_LeftView, UT_LeftView_closeEditor_001)
{
    LeftView leftview;
    QWidget *editor = new QWidget;
    leftview.closeEditor(editor, QAbstractItemDelegate::NoHint);
    EXPECT_TRUE(!editor->isVisible());
    delete editor;
}

TEST_F(UT_LeftView, UT_LeftView_doMove_001)
{
    LeftView leftview;
    QModelIndexList indexList;
    VNOTE_FOLDERS_MAP *folders = VNoteDataManager::instance()->getNoteFolders();
    if (folders) {
        for (auto it : folders->folders) {
            leftview.appendFolder(it);
        }
    }
    QModelIndex index = leftview.setDefaultNotepadItem();
    QString sortString = leftview.getFolderSort();
    leftview.doDragMove(index, index.siblingAtRow(index.row() + 1));
    leftview.setFolderSort();
    EXPECT_TRUE(sortString != leftview.getFolderSort());
}

TEST_F(UT_LeftView, UT_LeftView_getFolderSort_001)
{
    LeftView leftview;
    VNOTE_FOLDERS_MAP *folders = VNoteDataManager::instance()->getNoteFolders();
    if (folders) {
        for (auto it : folders->folders) {
            leftview.appendFolder(it);
        }
    }
    EXPECT_TRUE(!leftview.getFolderSort().isEmpty());
    EXPECT_EQ(true, leftview.setFolderSort());
}

TEST_F(UT_LeftView, UT_LeftView_setDrawNotesNum_001)
{
    LeftView leftview;
    QLineEdit *lineedit = new QLineEdit();
    lineedit->setText("test2agrrserfgargafgasrfgargargarsgarefgaehbsrtbhaerfgaertfaerfgaerfarfarfa3qw4taerarfa");

    leftview.m_pItemDelegate->setDrawNotesNum(false);
    leftview.m_pItemDelegate->setDragState(false);
    EXPECT_EQ(false, leftview.m_pItemDelegate->m_draging);
    leftview.m_pItemDelegate->setDrawHover(false);
    EXPECT_EQ(false, leftview.m_pItemDelegate->m_drawHover);
    leftview.m_pItemDelegate->setModelData(lineedit, leftview.m_pDataModel, leftview.currentIndex());
    delete lineedit;
}

TEST_F(UT_LeftView, UT_LeftView_eventFilter_001)
{
    LeftView leftview;
    QFocusEvent event(QEvent::FocusOut);
    leftview.eventFilter(&leftview, &event);
    EXPECT_EQ(false, leftview.m_pItemDelegate->m_tabFocus);
    QFocusEvent event1(QEvent::FocusIn);
    event1.m_reason = Qt::TabFocusReason;
    leftview.eventFilter(&leftview, &event1);
    EXPECT_EQ(true, leftview.m_pItemDelegate->m_tabFocus);
    QEvent event2(QEvent::DragLeave);
    leftview.eventFilter(nullptr, &event2);
    EXPECT_EQ(false, leftview.m_pItemDelegate->m_drawHover);
    QEvent event3(QEvent::DragEnter);
    leftview.eventFilter(nullptr, &event3);
    EXPECT_EQ(true, leftview.m_pItemDelegate->m_drawHover);
}

TEST_F(UT_LeftView, UT_LeftView_sort_001)
{
    LeftView leftview;
    VNOTE_FOLDERS_MAP *folders = VNoteDataManager::instance()->getNoteFolders();
    if (folders) {
        for (auto it : folders->folders) {
            leftview.appendFolder(it);
        }
    }
    leftview.sort();
    EXPECT_TRUE(!leftview.getFolderSort().isEmpty());
}

TEST_F(UT_LeftView, UT_LeftView_handleTouchSlideEvent_001)
{
    LeftView leftview;
    QPoint point(0, 10);
    leftview.handleTouchSlideEvent(10, 5.0, point);
    EXPECT_EQ(point, leftview.m_touchPressPoint);
}

TEST_F(UT_LeftView, UT_LeftView_dragEvent_001)
{
    LeftView leftview;
    QMimeData data;
    QDragEnterEvent event(QPoint(0, 0), Qt::IgnoreAction, &data, Qt::LeftButton, Qt::NoModifier);
    leftview.dragEnterEvent(&event);
    data.setData(NOTEPAD_DRAG_KEY, QByteArray());
    leftview.m_folderDraing = true;
    leftview.dragEnterEvent(&event);
    leftview.dragMoveEvent(&event);
    EXPECT_EQ(true, leftview.m_pItemDelegate->m_draging);
    QDragLeaveEvent event1;
    leftview.dragLeaveEvent(&event1);
    EXPECT_EQ(false, leftview.m_pItemDelegate->m_draging);
}
