// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QProcess>
#include <QDebug>

#include "../stub.h"

#include "../deb-installer/uab/uab_process_controller.h"
#include "../deb-installer/process/Pty.h"

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

void utDebProcessController::TearDown() {}

bool stub_isValid_true()
{
    return true;
}

bool stub_installBackendCliImpl_true(const Uab::UabPackage::Ptr &)
{
    return true;
}

TEST_F(utDebProcessController, installExecSuccess)
{
    Stub s;
    s.set(ADDR(Uab::UabPackage, isValid), stub_isValid_true);
    s.set(ADDR(Uab::UabProcessController, installBackendCliImpl), stub_installBackendCliImpl_true);

    Uab::UabProcessController uabController;
    auto uabPtr = Uab::UabPkgInfo::Ptr::create();
    uabPtr->filePath = "localtest";
    uabController.reset();
    uabController.markInstall(Uab::UabPackage::fromInfo(uabPtr));
    EXPECT_TRUE(uabController.commitChanges());

    EXPECT_TRUE(uabController.m_procFlag.testFlag(Uab::UabProcessController::Processing));
}
