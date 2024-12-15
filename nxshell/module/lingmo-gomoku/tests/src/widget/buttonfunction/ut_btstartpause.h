// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_BTSTARTPAUSE_H
#define UT_BTSTARTPAUSE_H

#include "gtest/gtest.h"
#include "btstartpause.h"

#include <QDebug>

class UT_BTStartPause : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ButtonStartPause = new BTStartPause();
        m_scene = new QGraphicsScene(0, 0, widgetWidth, widgetHeight - 50);
        m_scene->addItem(m_ButtonStartPause);
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        m_scene->removeItem(m_ButtonStartPause);
        delete m_ButtonStartPause;
        delete m_scene;
        qInfo() << "TearDown" << endl;
    }
    BTStartPause *m_ButtonStartPause;
    QGraphicsScene *m_scene;
};

#endif // UT_BTSTARTPAUSE_H
