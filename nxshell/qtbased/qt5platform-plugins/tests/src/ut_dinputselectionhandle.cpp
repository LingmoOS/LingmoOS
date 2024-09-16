// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWindow>
#include <QGuiApplication>

#include "dinputselectionhandle.h"
#include "ddesktopinputselectioncontrol.h"

DPP_USE_NAMESPACE

class TDInputSelectionHandle : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    DInputSelectionHandle *handle = nullptr;
    DDesktopInputSelectionControl *control = nullptr;
};

void TDInputSelectionHandle::SetUp()
{
    control = new DDesktopInputSelectionControl(nullptr, qApp->inputMethod());
    handle = new DInputSelectionHandle(DInputSelectionHandle::Up, control);
}

void TDInputSelectionHandle::TearDown()
{
    delete handle;
    delete control;
}

TEST_F(TDInputSelectionHandle, handlePosition)
{
    ASSERT_EQ(handle->handlePosition(), DInputSelectionHandle::Up);
}

TEST_F(TDInputSelectionHandle, setHandlePosition)
{
    handle->setHandlePosition(DInputSelectionHandle::Down);
    ASSERT_EQ(handle->handlePosition(), DInputSelectionHandle::Down);
    handle->setHandlePosition(DInputSelectionHandle::Up);
    ASSERT_EQ(handle->handlePosition(), DInputSelectionHandle::Up);
}

TEST_F(TDInputSelectionHandle, handleImageSize)
{
    auto size = handle->handleImageSize();
    ASSERT_TRUE(!size.isNull());
}
