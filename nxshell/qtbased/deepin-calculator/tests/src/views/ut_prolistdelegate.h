// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PROLISTDELEGATE_H
#define UT_PROLISTDELEGATE_H

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QTest>

#include <QPainter>
#include "../../src/views/prolistdelegate.h"
#include "../../src/views/prolistmodel.h"
#include "../../src/views/prolistview.h"
#include"../stub.h"

class Ut_ProListDelegate : public ::testing::Test
{
public:
    Ut_ProListDelegate();

    ProListView *m_prolistview1;
    ProListModel *m_model1;
    ProListDelegate *m_proListDelegate1;
    ProListDelegate *m_proListDelegate;
    ProListModel *m_model;
protected:
    void SetUp();
    void TearDown();
};

#endif // UT_PROLISTDELEGATE_H
