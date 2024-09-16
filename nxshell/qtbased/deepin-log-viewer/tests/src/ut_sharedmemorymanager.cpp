// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharedmemorymanager.h"

#include <gtest/gtest.h>
#include <stub.h>

#include <QDebug>

TEST(SharedMemoryManager_Constructor_UT, SharedMemoryManager_Constructor_UT)
{
    SharedMemoryManager *p = new SharedMemoryManager(nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(SharedMemoryManager_setRunnableTag_UT, SharedMemoryManager_setRunnableTag_UT)
{
    SharedMemoryManager *p = new SharedMemoryManager(nullptr);
    EXPECT_NE(p, nullptr);
    ShareMemoryInfo info;
    info.isStart = true;
    p->setRunnableTag(info);
    p->deleteLater();
}

TEST(SharedMemoryManager_getRunnableKey_UT, SharedMemoryManager_getRunnableKey_UT)
{
    SharedMemoryManager *p = new SharedMemoryManager(nullptr);
    EXPECT_NE(p, nullptr);
    p->getRunnableKey();
    p->deleteLater();
}

TEST(SharedMemoryManager_isAttached_UT, SharedMemoryManager_isAttached_UT)
{
    SharedMemoryManager *p = new SharedMemoryManager(nullptr);
    EXPECT_NE(p, nullptr);
    p->isAttached();
    p->deleteLater();
}

TEST(SharedMemoryManager_init_UT, SharedMemoryManager_init_UT)
{
    SharedMemoryManager *p = new SharedMemoryManager(nullptr);
    EXPECT_NE(p, nullptr);
    p->init();
    p->deleteLater();
}
