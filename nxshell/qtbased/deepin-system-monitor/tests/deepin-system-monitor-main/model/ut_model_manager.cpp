// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "model/model_manager.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/
class UT_ModelManager: public ::testing::Test
{
public:
    UT_ModelManager() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ModelManager(nullptr);
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    ModelManager *m_tester;
};

TEST_F(UT_ModelManager, initTest)
{

}


TEST_F(UT_ModelManager, test_instance_001)
{
    m_tester->instance();

}
