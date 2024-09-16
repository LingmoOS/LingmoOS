// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_PLAYINGSCREEN_H
#define UT_PLAYINGSCREEN_H

#include <QObject>
#include <QGraphicsScene>
#include <QDebug>
#include "constants.h"
#include "gtest/gtest.h"

#include "playingscreen.h"

class UT_PlayingScreen : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_PlayingScreen = new PlayingScreen();
        m_scene = new QGraphicsScene(0, 0, widgetWidth, widgetHeight - 50);
        m_scene->addItem(m_PlayingScreen);
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        m_scene->removeItem(m_PlayingScreen);
        delete m_PlayingScreen;
        delete m_scene;
        qInfo() << "TearDown" << endl;
    }
    PlayingScreen *m_PlayingScreen;
    QGraphicsScene *m_scene;
};

#endif // UT_PLAYINGSCREEN_H
