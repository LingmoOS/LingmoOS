// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QProcess>
#include <QDebug>

#include "../stub.h"

#include "../deb-installer/uab/uab_process_controller.h"

class utDebProcessController : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

private:
    Stub utStub;
};

void stub_QProcess_start(QProcess::OpenMode mode)
{
    // do nothing
}

void utDebProcessController::SetUp()
{
    utStub.set((void(QProcess::*)(QProcess::OpenMode))ADDR(QProcess, start), stub_QProcess_start);
}

void utDebProcessController::TearDown() { }

TEST_F(utDebProcessController, installExecSuccess)
{
    Uab::UabProcessController uabController;
    auto uabPtr = Uab::UabPkgInfo::Ptr::create();
    uabPtr->filePath = "localtest";
    uabController.install(uabPtr);

    EXPECT_EQ(uabController.m_process->program(), uabPtr->filePath);
    EXPECT_TRUE(uabController.m_procFlag.testFlag(Uab::UabProcessController::Installing));
}
