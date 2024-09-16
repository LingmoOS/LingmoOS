// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_leftviewdelegate.h"
#include "leftviewdelegate.h"
#include "leftview.h"

#include "common/vnotedatamanager.h"
#include "common/standarditemcommon.h"

#include <QLineEdit>
#include <QStyleOptionViewItem>
#include <QPainter>

UT_LeftViewDelegate::UT_LeftViewDelegate()
{
}

TEST_F(UT_LeftViewDelegate, UT_LeftViewDelegate_paintNoteItem_001)
{
    LeftView view;
    for (auto it : VNoteDataManager::instance()->getNoteFolders()->folders) {
        view.addFolder(it);
    }
    view.setDefaultNotepadItem();
    LeftViewDelegate *delegate = view.m_pItemDelegate;
    QLineEdit edit;
    edit.setText(QString("111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"));
    delegate->setModelData(&edit, view.model(), view.currentIndex());
    QStyleOptionViewItem option;
    option.state.setFlag(QStyle::State_MouseOver, true);
    option.rect = QRect(0, 0, 40, 30);
    delegate->setDragState(true);
    EXPECT_EQ(true, delegate->m_draging);
    QPainter paint;
    delegate->paintNoteItem(&paint, option, view.currentIndex());
    delegate->setEnableItem(true);
    EXPECT_EQ(true, delegate->m_enableItem);
    option.state.setFlag(QStyle::State_Enabled, true);
    delegate->paintNoteItem(&paint, option, view.currentIndex());
    delegate->setDragState(false);
    EXPECT_EQ(false, delegate->m_draging);
    delegate->paintNoteItem(&paint, option, view.currentIndex());
    delegate->setDrawHover(true);
    EXPECT_EQ(true, delegate->m_drawHover);
    delegate->paintNoteItem(&paint, option, view.currentIndex());
    delegate->paintTabFocusBackground(&paint, option, option.rect);
}
