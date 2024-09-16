// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_BTMUSICCONTROL_H
#define UT_BTMUSICCONTROL_H

#include "gtest/gtest.h"
#include "btmusiccontrol.h"

#include <QDebug>

class UT_BTMusicControl : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ButtonMusicControl = new BTMusicControl();
        m_scene = new QGraphicsScene(0, 0, widgetWidth, widgetHeight - 50);
        m_scene->addItem(m_ButtonMusicControl);
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        m_scene->removeItem(m_ButtonMusicControl);
        delete m_ButtonMusicControl;
        delete m_scene;
        qInfo() << "TearDown" << endl;
    }
    BTMusicControl *m_ButtonMusicControl;
    QGraphicsScene *m_scene;
};

#endif // UT_BTMUSICCONTROL_H
