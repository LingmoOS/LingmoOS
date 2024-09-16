// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QtTest>
#include <QColor>
#include <QDebug>
#include <gtest/gtest.h>
#include "../src/utils/shapesutils.h"



using namespace testing;

class ToolshapeTest:public testing::Test, public QObject{

public:

    int m_fontsize;
    Toolshape *toolshape;
    virtual void SetUp() override{
        std::cout << "start TextButtonTest" << std::endl;
        toolshape = new Toolshape;
    }

    virtual void TearDown() override{
        delete toolshape;
        std::cout << "end TextButtonTest" << std::endl;
    }
};
