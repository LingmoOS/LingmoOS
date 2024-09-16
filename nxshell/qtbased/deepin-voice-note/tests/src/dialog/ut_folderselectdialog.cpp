// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_folderselectdialog.h"
#include "folderselectdialog.h"
#include "leftview.h"
#include "leftviewdelegate.h"
#include "leftviewsortfilter.h"
#include "common/vnoteforlder.h"

#include <QStandardItemModel>

UT_FolderSelectDialog::UT_FolderSelectDialog()
{
}

TEST_F(UT_FolderSelectDialog, UT_FolderSelectDialog_setNoteContext_001)
{
    QStandardItemModel data;
    FolderSelectDialog folderselectdialog(&data);
    folderselectdialog.setNoteContextInfo("test", 1);
    EXPECT_EQ(Qt::AlignCenter, folderselectdialog.m_noteInfo->alignment()) << "alignment";
    EXPECT_EQ("test", folderselectdialog.m_notesName) << "m_notesName";
    EXPECT_EQ(1, folderselectdialog.m_notesNumber) << "m_notesNumber";
    EXPECT_EQ(-1, folderselectdialog.getSelectIndex().row());
}

TEST_F(UT_FolderSelectDialog, UT_FolderSelectDialog_clearSelection_001)
{
    QStandardItemModel data;
    FolderSelectDialog folderselectdialog(&data);
    folderselectdialog.clearSelection();
    EXPECT_FALSE(folderselectdialog.m_confirmBtn->isEnabled());
}

TEST_F(UT_FolderSelectDialog, UT_FolderSelectDialog_hideEvent)
{
    QStandardItemModel data;
    FolderSelectDialog folderselectdialog(&data);
    QHideEvent *event = new QHideEvent();
    folderselectdialog.hideEvent(event);
    EXPECT_FALSE(folderselectdialog.m_closeButton->testAttribute(Qt::WA_UnderMouse)) << "attribute";
    EXPECT_FALSE(folderselectdialog.m_view->hasFocus()) << "hasfocus";
    delete event;
}

TEST_F(UT_FolderSelectDialog, UT_FolderSelectDialog_refreshTextColor_001)
{
    QStandardItemModel dataModel;
    FolderSelectDialog folderselectdialog(&dataModel);
    QColor colorDark = QColor(255, 255, 255);
    folderselectdialog.refreshTextColor(true);
    EXPECT_EQ(colorDark, folderselectdialog.m_labMove->palette().windowText().color())
        << "dark is true, m_view color";
    colorDark.setAlphaF(0.7);
    EXPECT_EQ(colorDark, folderselectdialog.m_noteInfo->palette().windowText().color())
        << "dark is true, m_noteInfo color";
    QColor color = QColor(0, 0, 0, 1);
    color.setAlphaF(0.9);
    folderselectdialog.refreshTextColor(false);
    EXPECT_EQ(color, folderselectdialog.m_labMove->palette().windowText().color())
        << "dark is false, m_view color";
    color.setAlphaF(0.7);
    EXPECT_EQ(color, folderselectdialog.m_noteInfo->palette().windowText().color())
        << "dark is false, m_noteInfo color";
}

TEST_F(UT_FolderSelectDialog, UT_FolderSelectDialog_setFolderBlack_001)
{
    VNoteFolder *folder1 = new VNoteFolder;
    QStandardItemModel dataModel;
    FolderSelectDialog folderselectdialog(&dataModel);
    QList<VNoteFolder *> blackList;
    blackList.push_back(folder1);
    folderselectdialog.setFolderBlack(blackList);
    EXPECT_EQ(blackList, folderselectdialog.m_model->m_blackFolders);
    delete folder1;
}
