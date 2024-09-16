// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "AttrScrollWidget.h"
#include "DocSheet.h"
#include "Global.h"

#include "stub.h"

#include <gtest/gtest.h>
#include <QTest>

class TestAttrScrollWidget : public ::testing::Test
{
public:
    TestAttrScrollWidget(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", m_tester);
        m_tester = new AttrScrollWidget(sheet, m_tester);
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete sheet;
        delete m_tester;
    }

protected:
    DocSheet *sheet = nullptr;
    AttrScrollWidget *m_tester = nullptr;
};

TEST_F(TestAttrScrollWidget, initTest)
{

}
