// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_prolistmodel.h"

#include "../../3rdparty/math/quantity.h"

Ut_ProListModel::Ut_ProListModel()
{

}

void Ut_ProListModel::SetUp()
{
    m_proListModel = new ProListModel();
}

void Ut_ProListModel::TearDown()
{
    delete m_proListModel;
}

TEST_F(Ut_ProListModel, data)
{
    m_proListModel->updataList(Quantity(3));
    m_proListModel->data(m_proListModel->index(1, 0), ProListModel::Role::ExpressionCount);
    m_proListModel->data(m_proListModel->index(1, 0), ProListModel::Role::ExpressionPrevious);
    EXPECT_EQ(m_proListModel->data(m_proListModel->index(1, 0), ProListModel::Role::ExpressionNext), "3");
}

TEST_F(Ut_ProListModel, clearItems)
{
    m_proListModel->clearItems();
    EXPECT_EQ(m_proListModel->m_expressionList.at(3), "");
}

TEST_F(Ut_ProListModel, updataList)
{
    m_proListModel->updataList(Quantity(11));
    EXPECT_TRUE(m_proListModel->m_expressionList.at(0) == "B");
    EXPECT_TRUE(m_proListModel->m_expressionList.at(1) == "11");
    EXPECT_TRUE(m_proListModel->m_expressionList.at(2) == "13");
    EXPECT_TRUE(m_proListModel->m_expressionList.at(3) == "1011");
}


