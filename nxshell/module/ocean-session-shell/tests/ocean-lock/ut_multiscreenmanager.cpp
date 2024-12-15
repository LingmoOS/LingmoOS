// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multiscreenmanager.h"

#include <QGuiApplication>

#include <gtest/gtest.h>

class UT_MultiScreenManager : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    MultiScreenManager *m_manager;
};

void UT_MultiScreenManager::SetUp()
{
    m_manager = new MultiScreenManager;
}

void UT_MultiScreenManager::TearDown()
{
    delete m_manager;
}

TEST_F(UT_MultiScreenManager, basic)
{
    m_manager->startRaiseContentFrame();
    m_manager->register_for_mutil_screen(nullptr);
    m_manager->onScreenRemoved(QGuiApplication::primaryScreen());
}
