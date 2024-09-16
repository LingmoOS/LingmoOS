// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "smooth_curve_generator.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <DApplication>

/***************************************STUB begin*********************************************/
/***************************************STUB end**********************************************/


class UT_SmoothCurveGenerator: public ::testing::Test
{
public:
    UT_SmoothCurveGenerator() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SmoothCurveGenerator();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    SmoothCurveGenerator *m_tester;
};

TEST_F(UT_SmoothCurveGenerator, initTest)
{
}

TEST_F(UT_SmoothCurveGenerator, test_onDetailInfoClicked)
{
}
