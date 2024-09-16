// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWindow>
#include <QDebug>

#include "utility.h"
#include <xcb/xcb.h>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QtX11Extras/QX11Info>
#endif

TEST(TUtility, dropShadow)
{
    QPixmap pixmap;
    QImage image = Utility::dropShadow(pixmap, 4, QColor(255, 255, 0));
    ASSERT_TRUE(image.isNull());
    pixmap = QPixmap(100, 100);
    image = Utility::dropShadow(pixmap, 4, QColor(255, 255, 0));
    ASSERT_FALSE(image.isNull());
    ASSERT_TRUE(image.size() == QSize(100 + 2 * 4, 100 + 2 * 4));
}

TEST(TUtility, borderImage)
{
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::red);
    QImage image = Utility::borderImage(pixmap, QMargins(4, 4, 4, 4), QSize(200, 200));
    ASSERT_FALSE(image.isNull());
    ASSERT_TRUE(image.size() == QSize(200, 200));
}

TEST(TUtility, sudokuByRect)
{
    QRect rect(0, 0, 100, 100);
    QMargins margins(4, 4, 4, 4);
    auto rects = Utility::sudokuByRect(rect, margins);
    ASSERT_TRUE(rects.size() == 9);
    ASSERT_TRUE(rects[0] == QRect(0, 0, 4, 4));
    ASSERT_TRUE(rects[8] == QRect(96, 96, 4, 4));
}

TEST(TUtility, internAtom)
{
    xcb_atom_t atom = Utility::internAtom("");
    ASSERT_TRUE(atom == XCB_NONE);
    atom = Utility::internAtom("test");
    ASSERT_TRUE(atom == XCB_NONE);
    atom = Utility::internAtom(QX11Info::connection(), "test");
    ASSERT_TRUE(atom == XCB_NONE);
}
