// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_CHECKERBOARDITME_H
#define UT_CHECKERBOARDITME_H

#include "gtest/gtest.h"
#include "checkerboarditem.h"
#include "checkerboardscene.h"

#include <QObject>
#include <QDebug>

class UT_CheckerboardItme : public testing::Test
{
public:
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_checkerboardItem = new CheckerboardItem();
        m_checkerboardScene = new QGraphicsScene(0, 0, widgetWidth, widgetHeight - 50);
        m_checkerboardScene->addItem(m_checkerboardItem);
        qInfo() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        m_checkerboardScene->removeItem(m_checkerboardItem);
        delete m_checkerboardItem;
        delete m_checkerboardScene;
        qInfo() << "TearDown" << endl;
    }
    CheckerboardItem *m_checkerboardItem;
    QGraphicsScene *m_checkerboardScene;
};

#endif // UT_CHECKERBOARDITME_H
