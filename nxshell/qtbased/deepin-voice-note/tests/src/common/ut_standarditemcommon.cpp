// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_standarditemcommon.h"
#include "standarditemcommon.h"

UT_StandardItemCommon::UT_StandardItemCommon()
{
}

void UT_StandardItemCommon::SetUp()
{
    m_standarditemcommon = new StandardItemCommon();
}

void UT_StandardItemCommon::TearDown()
{
    delete m_standarditemcommon;
}

TEST_F(UT_StandardItemCommon, UT_StandardItemCommon_getStandardItemType_001)
{
    QStandardItemModel *pDataModel = new QStandardItemModel();
    QStandardItem *pItem = pDataModel->item(0);
    if (pItem == nullptr) {
        pItem = m_standarditemcommon->createStandardItem(nullptr, StandardItemCommon::NOTEPADROOT);
        pDataModel->insertRow(0, pItem);
    }

    QModelIndex index = pDataModel->index(0, 0);
    int ret = m_standarditemcommon->getStandardItemType(index);
    EXPECT_EQ(ret, StandardItemCommon::StandardItemType::NOTEPADROOT) << "getStandardItemType, index(0, 0)";
    EXPECT_FALSE(m_standarditemcommon->getStandardItemData(index)) << "getStandardItemData, index(0, 0)";

    index = pDataModel->index(0, 1);
    ret = m_standarditemcommon->getStandardItemType(index);
    EXPECT_EQ(ret, StandardItemCommon::StandardItemType::Invalid) << "getStandardItemType, index(0, 0)";
    EXPECT_FALSE(m_standarditemcommon->getStandardItemData(index)) << "getStandardItemData, index(0, 1)";
    delete pDataModel;
}
