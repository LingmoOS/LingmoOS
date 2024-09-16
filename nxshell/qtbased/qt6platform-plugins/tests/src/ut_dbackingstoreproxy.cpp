// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWindow>
#include <QVariant>

#include "dbackingstoreproxy.h"

DPP_USE_NAMESPACE

class TDBackingStoreProxy : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    QWindow *window;
};

void TDBackingStoreProxy::SetUp()
{
    window = new QWindow;
}

void TDBackingStoreProxy::TearDown()
{
    delete window;
}

TEST_F(TDBackingStoreProxy, useGLPaint)
{
    ASSERT_FALSE(DBackingStoreProxy::useGLPaint(window));
    window->setSurfaceType(QSurface::OpenGLSurface);
    window->setProperty("_d_enableGLPaint", QVariant::fromValue(true));
    ASSERT_TRUE(DBackingStoreProxy::useGLPaint(window));
}

TEST_F(TDBackingStoreProxy, useWallpaperPaint)
{
    ASSERT_FALSE(DBackingStoreProxy::useWallpaperPaint(window));
    window->setProperty("_d_dxcb_wallpaper", QVariant::fromValue(true));
    ASSERT_TRUE(DBackingStoreProxy::useWallpaperPaint(window));
}
