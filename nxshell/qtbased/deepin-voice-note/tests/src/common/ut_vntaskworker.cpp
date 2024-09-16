// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vntaskworker.h"
#include "vntaskworker.h"
#include "filecleanupworker.h"

UT_VNTaskWorker::UT_VNTaskWorker()
{
}

void UT_VNTaskWorker::SetUp()
{
    m_vntaskworker = new VNTaskWorker;
}

void UT_VNTaskWorker::TearDown()
{
    delete m_vntaskworker;
}

TEST_F(UT_VNTaskWorker, UT_VNTaskWorker_addTask_001)
{
    m_vntaskworker->addTask(nullptr);
}

TEST_F(UT_VNTaskWorker, UT_VNTaskWorker_setWorkerName_001)
{
    QString worke = "aaa";
    m_vntaskworker->setWorkerName(worke);
    EXPECT_EQ("aaa", m_vntaskworker->m_workerName);
}

TEST_F(UT_VNTaskWorker, UT_VNTaskWorker_quitWorker_001)
{
    m_vntaskworker->quitWorker();
    EXPECT_TRUE(m_vntaskworker->m_fQuit);
}

TEST_F(UT_VNTaskWorker, UT_VNTaskWorker_run_001)
{
    m_vntaskworker->m_fQuit = true;
    m_vntaskworker->run();
}
