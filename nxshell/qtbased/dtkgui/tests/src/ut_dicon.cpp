// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"

#include <QGuiApplication>
#include <DIcon>

DGUI_USE_NAMESPACE

TEST(ut_DIcon, pixmap)
{
    QIcon icon = QIcon::fromTheme("dde", QIcon(":/images/logo_icon.png"));
    ASSERT_FALSE(icon.isNull());

    DIcon dicon(icon);
    QSize size(32, 32);

    qreal devicePixelRatio = qApp->devicePixelRatio();
    // D_DXCB_DISABLE_OVERRIDE_HIDPI=1 QT_SCALE_FACTOR=1.25
    ASSERT_FLOAT_EQ(devicePixelRatio, 1.25);

    // 默认单元测试未开启 AA_UseHighDpiPixmaps， 这时 pixmap 获取的 devicePixelRatio 为 1.0
    // 图片大小也就是设置大小 32x32
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    ASSERT_FALSE(QGuiApplication::testAttribute(Qt::AA_UseHighDpiPixmaps));
#endif

    ASSERT_EQ(icon.pixmap(size).size(), size);

    // 开启 AA_UseHighDpiPixmaps 之后 pixmap 的大小和 qApp 缩放有关
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    ASSERT_EQ(icon.pixmap(size).size(), size * devicePixelRatio);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, false);

    ASSERT_EQ(dicon.pixmap(size, -1).size(), size * devicePixelRatio);

    ASSERT_EQ(dicon.pixmap(size, 1.0).size(), size);

    ASSERT_EQ(dicon.pixmap(size, 1.75).size(), size * 1.75);
#else
    // DIcon 在 Qt6 下行为应和 QIcon 这个接口一致
    ASSERT_EQ(dicon.pixmap(size, -1).size(), icon.pixmap(size, -1).size());

    ASSERT_EQ(dicon.pixmap(size, 1.0).size(), icon.pixmap(size, 1.0).size());

    ASSERT_EQ(dicon.pixmap(size, 1.75).size(), icon.pixmap(size, 1.75).size());
#endif


}
