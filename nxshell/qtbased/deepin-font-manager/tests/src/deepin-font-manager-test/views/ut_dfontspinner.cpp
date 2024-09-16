// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/dfontspinner.h"


#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "commonheaderfile.h"
#include "utils.h"

#include <DFontSizeManager>

#include <QPaintEvent>
#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QPainterPath>

namespace {
class TestDFontSpinner : public testing::Test
{

protected:
    void SetUp()
    {
        fs = new DFontSpinner(w);
    }
    void TearDown()
    {
        delete fs;
        delete w;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFontSpinner *fs;
};
static QString g_funcname;
void stub_start(void *)
{
    g_funcname = __FUNCTION__;
}

void stub_stop()
{
    g_funcname = __FUNCTION__;
}
void stub_addEllipse(void *, const QRectF &)
{
    g_funcname = __FUNCTION__;
}
void stub_clear(void *)
{
    g_funcname = __FUNCTION__;
}
}


TEST_F(TestDFontSpinner, checkStart)
{
    Stub s;
    s.set((void(QTimer::*)())ADDR(QTimer, start), stub_start);

    fs->start();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_start"));
}

TEST_F(TestDFontSpinner, checkPaintEvent)
{
    Stub s;
    s.set((void(QPainterPath::*)(const QRectF &))ADDR(QPainterPath, addEllipse), stub_addEllipse);

    QRect f;
    QPaintEvent *paint = new QPaintEvent(f);
    fs->paintEvent(paint);
    SAFE_DELETE_ELE(paint)
    EXPECT_TRUE(g_funcname == QLatin1String("stub_addEllipse"));
}


TEST_F(TestDFontSpinner, checkChangeEvent)
{
    Stub s;
    s.set((void(QList<QList<QColor>>::*)())ADDR(QList<QList<QColor>>, clear), stub_clear);

    QEvent *e = new QEvent(QEvent::PaletteChange);
    fs->changeEvent(e);
    SAFE_DELETE_ELE(e)
    EXPECT_TRUE(g_funcname == QLatin1String("stub_clear"));
}



