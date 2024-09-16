// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_GAMECONTROL_H
#define UT_GAMECONTROL_H

#include "gtest/gtest.h"
#include "gamecontrol.h"
#include <QDebug>

class UT_GameControl : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_GameControl = new GameControl(chess_white, chess_black);
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        if (m_GameControl != nullptr) {
            delete m_GameControl;
            m_GameControl = nullptr;
        }
        qInfo() << "TearDown" << endl;
    }
    GameControl *m_GameControl;
};

#endif // UT_GAMECONTROL_H
