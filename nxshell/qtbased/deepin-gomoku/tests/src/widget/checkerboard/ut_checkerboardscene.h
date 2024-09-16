// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CHECKERBOARDSCENE_H
#define UT_CHECKERBOARDSCENE_H

#include "gtest/gtest.h"
#include "checkerboardscene.h"

#include <QDebug>

class UT_CheckerboardScene : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_checkerboardScene = new CheckerboardScene(0, 0, widgetWidth, widgetHeight - 50);
        m_checkerboardScene->initChess();
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        if (m_checkerboardScene != nullptr) {
            delete m_checkerboardScene;
            m_checkerboardScene = nullptr;
        }
        qInfo() << "TearDown" << endl;
    }
    CheckerboardScene *m_checkerboardScene;
};

#endif // UT_CHECKERBOARDSCENE_H
