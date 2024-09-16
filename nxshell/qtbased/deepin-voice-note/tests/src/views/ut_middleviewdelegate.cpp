// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_middleviewdelegate.h"
#include "middleview.h"
#include "common/vnotedatamanager.h"
#include "common/vnoteitem.h"
#include "db/vnoteitemoper.h"
#include "common/vnoteforlder.h"
#include "middleviewdelegate.h"

#include <QLineEdit>
#include <QPainter>

UT_MiddleViewDelegate::UT_MiddleViewDelegate(QObject *parent)
    : QObject(parent)
{
}

TEST_F(UT_MiddleViewDelegate, UT_MiddleViewDelegate_ModifyTextAndPaint_001)
{
    VNoteFolder *folder = VNoteDataManager::instance()->getNoteFolders()->folders[0];
    VNoteItemOper noteOper;
    VNOTE_ITEMS_MAP *notes = noteOper.getFolderNotes(folder->id);
    MiddleView view;
    VNoteItem *data = nullptr;
    if (notes) {
        notes->lock.lockForRead();
        for (auto it : notes->folderNotes) {
            view.appendRow(it);
            data = it;
            break;
        }
        notes->lock.unlock();
    }
    EXPECT_TRUE(data != nullptr);

    view.setCurrentIndex(0);
    EXPECT_TRUE(view.rowCount() != 0);
    MiddleViewDelegate *delegate = view.m_pItemDelegate;
    QStyleOptionViewItem option;
    option.state.setFlag(QStyle::State_Enabled, true);
    option.rect = QRect(0, 0, 140, 84);
    QLineEdit *edit = qobject_cast<QLineEdit *>(delegate->createEditor(&view, option, view.currentIndex()));
    EXPECT_TRUE(edit != nullptr);
    delegate->updateEditorGeometry(edit, option, view.currentIndex());
    delegate->setEditorData(edit, view.currentIndex());
    if(nullptr != data){
        EXPECT_EQ(edit->text(), data->noteTitle);
    }
    edit->setText(QString("111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"));
    delegate->setModelData(edit, view.model(), view.currentIndex());

    QPainter paint;
    bool isSelect = false;
    delegate->paintItemBase(&paint, option, option.rect, isSelect);
    option.state.setFlag(QStyle::State_MouseOver, true);
    delegate->paintItemBase(&paint, option, option.rect, isSelect);
    delegate->setEnableItem(false);
    delegate->paintItemBase(&paint, option, option.rect, isSelect);
    option.state.setFlag(QStyle::State_Selected, true);
    delegate->paintItemBase(&paint, option, option.rect, isSelect);
    delegate->setTabFocus(true);
    delegate->paintItemBase(&paint, option, option.rect, isSelect);
}
