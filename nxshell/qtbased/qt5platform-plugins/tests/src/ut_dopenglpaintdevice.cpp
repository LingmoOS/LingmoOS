// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWindow>
#include <QPaintEngine>
#include <QOpenGLFramebufferObject>
#include <QDebug>

#include "dopenglpaintdevice.h"

DPP_USE_NAMESPACE

static void drawColoredRects(QPainter *p, const QSize &size)
{
    p->fillRect(0, 0, size.width() / 2, size.height() / 2, Qt::red);
    p->fillRect(size.width() / 2, 0, size.width() / 2, size.height() / 2, Qt::green);
    p->fillRect(size.width() / 2, size.height() / 2, size.width() / 2, size.height() / 2, Qt::blue);
    p->fillRect(0, size.height() / 2, size.width() / 2, size.height() / 2, Qt::white);
}

TEST(TDOpenGLPaintDevice, OpenGLPaintDevice)
{
    const QSize size(128, 128);

    DOpenGLPaintDevice device(size);
    QPainter p;
    ASSERT_TRUE(p.begin(&device));
    drawColoredRects(&p, device.size());
    p.end();
    ASSERT_TRUE(device.isValid());
    ASSERT_EQ(device.grabFramebuffer().size(), size);
}
