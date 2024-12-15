// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_BTREPLAY_H
#define UT_BTREPLAY_H

#include "gtest/gtest.h"
#include "btreplay.h"

#include <QDebug>

class UT_BTReplay : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_ButtonReplay = new BTReplay();
        m_scene = new QGraphicsScene(0, 0, widgetWidth, widgetHeight - 50);
        m_scene->addItem(m_ButtonReplay);
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        m_scene->removeItem(m_ButtonReplay);
        delete m_ButtonReplay;
        delete m_scene;
        qInfo() << "TearDown" << endl;
    }
    BTReplay *m_ButtonReplay;
    QGraphicsScene *m_scene;
};

#endif // UT_BTREPLAY_H
