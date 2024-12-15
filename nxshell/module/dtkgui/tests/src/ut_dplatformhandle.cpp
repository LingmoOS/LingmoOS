// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWidget>
#include <QWindow>
#include <QTest>
#include <QDebug>
#include <QBuffer>

#include "dplatformhandle.h"
#include "dwindowmanagerhelper.h"

#define DXCB_PLUGIN_KEY "dxcb"
#define DXCB_PLUGIN_SYMBOLIC_PROPERTY "_d_isDxcb"
#define SETWMWALLPAPERPARAMETER "_d_setWmWallpaperParameter"
#define CLIENTLEADER "_d_clientLeader"
#define WINDOWRADIUS "_d_windowRadius"
#define BORDERWIDTH "_d_borderWidth"
#define BORDRCOLOR "_d_borderColor"
#define SHADOWRADIUS "_d_shadowRadius"
#define SHADOWOFFSET "_d_shadowOffset"
#define SHADOWCOLOR "_d_shadowColor"
#define CLIPPATH "_d_clipPath"
#define FRAMEMASK "_d_frameMask"
#define FRAMEMARGINS "_d_frameMargins"
#define TRANSLUCENTBACKGROUND "_d_translucentBackground"
#define ENABLESYSTEMRESIZE "_d_enableSystemResize"
#define ENABLESYSTEMMOVE "_d_enableSystemMove"
#define ENABLEBLURWINDOW "_d_enableBlurWindow"
#define AUTOINPUTMASKBYCLIPPATH "_d_autoINPUTMASKBYCLIPPATH"
#define REALWINDOWID "_d_real_content_window"

DGUI_USE_NAMESPACE

class TDPlatformHandle : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    QWindow *window;
    DPlatformHandle *pHandle;
};

void TDPlatformHandle::SetUp()
{
    window = new QWindow;
    window->create();

    pHandle = new DPlatformHandle(window);
    ASSERT_TRUE(pHandle);
}

void TDPlatformHandle::TearDown()
{
    delete window;
    delete pHandle;
}

TEST_F(TDPlatformHandle, testFunction)
{
    if (!pHandle || qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    qDebug() << "pluginVersion is " << DPlatformHandle::pluginVersion();

    EXPECT_EQ(DPlatformHandle::isDXcbPlatform(), (qApp->platformName() == DXCB_PLUGIN_KEY || qApp->property(DXCB_PLUGIN_SYMBOLIC_PROPERTY).toBool()));
    (DPlatformHandle::enableDXcbForWindow(window));
    (DPlatformHandle::enableDXcbForWindow(window, true));

    qInfo() << "TDPlatformHandle(isEnabledDXcb):" << DPlatformHandle::isEnabledDXcb(window);

    if (DPlatformHandle::isEnabledDXcb(window)) {
        EXPECT_TRUE(DPlatformHandle::setEnabledNoTitlebarForWindow(window, true));
        EXPECT_TRUE(DPlatformHandle::setEnabledNoTitlebarForWindow(window, false));
    }

    QVector<DPlatformHandle::WMBlurArea> wmAreaVector;
    wmAreaVector << dMakeWMBlurArea(0, 0, 20, 20, 4, 4);

    if (DWindowManagerHelper::instance()->hasBlurWindow()) {
        EXPECT_TRUE(pHandle->setWindowBlurAreaByWM(window, wmAreaVector));

        QPainterPath pPath;
        pPath.addRect({0, 0, 20, 20});
        EXPECT_TRUE(pHandle->setWindowBlurAreaByWM(window, {pPath}));
        EXPECT_TRUE(pHandle->setWindowBlurAreaByWM(wmAreaVector));
        EXPECT_TRUE(pHandle->setWindowBlurAreaByWM({pPath}));
    }

    if (qApp->platformFunction(SETWMWALLPAPERPARAMETER)) {
        EXPECT_TRUE(pHandle->setWindowWallpaperParaByWM(window, {0, 0, 20, 20}, DPlatformHandle::FollowScreen, DPlatformHandle::PreserveAspectFit));
    } else {
        EXPECT_FALSE(pHandle->setWindowWallpaperParaByWM(window, {0, 0, 20, 20}, DPlatformHandle::FollowScreen, DPlatformHandle::PreserveAspectFit));
    }


    if (qApp->platformFunction(CLIENTLEADER)) {
        ASSERT_TRUE(DPlatformHandle::windowLeader());
    } else {
        ASSERT_FALSE(DPlatformHandle::windowLeader());
    }

    DPlatformHandle::setDisableWindowOverrideCursor(window, true);
    QVariant windowRadius = window->property(WINDOWRADIUS);

    if (windowRadius.isValid() && windowRadius.canConvert(QVariant::Int)) {
        ASSERT_EQ(pHandle->windowRadius(), windowRadius.toInt());
    }

    QVariant borderWidth = window->property(BORDERWIDTH);

    if (borderWidth.isValid() && borderWidth.canConvert(QVariant::Int)) {
        ASSERT_EQ(pHandle->borderWidth(), borderWidth.toInt());
    } else {
        ASSERT_EQ(pHandle->borderWidth(), 0);
    }

    QVariant borderColor = window->property(BORDRCOLOR);

    if (borderColor.isValid() && borderColor.canConvert(QVariant::Color)) {
        ASSERT_EQ(pHandle->borderColor(), borderColor.value<QColor>());
    } else {
        ASSERT_FALSE(pHandle->borderColor().isValid());
    }

    QVariant shadowRadius = window->property(SHADOWRADIUS);

    if (shadowRadius.isValid() && shadowRadius.canConvert(QVariant::Int)) {
        ASSERT_EQ(pHandle->shadowRadius(), shadowRadius.toInt());
    } else {
        ASSERT_FALSE(pHandle->borderColor().isValid());
    }

    QVariant shadowOffset = window->property(SHADOWOFFSET);

    if (shadowOffset.isValid() && shadowOffset.canConvert(QVariant::Point)) {
        ASSERT_EQ(pHandle->shadowOffset(), shadowOffset.value<QPoint>());
    } else {
        ASSERT_TRUE(pHandle->shadowOffset().isNull());
    }

    QVariant shadowColor = window->property(SHADOWCOLOR);

    if (shadowColor.isValid() && shadowColor.canConvert(QVariant::Color)) {
        ASSERT_EQ(pHandle->shadowColor(), shadowColor.value<QColor>());
    } else {
        ASSERT_FALSE(pHandle->shadowColor().isValid());
    }

    QVariant clipPath = window->property(CLIPPATH);

    if (clipPath.isValid() && !clipPath.value<QPainterPath>().isEmpty()) {
        ASSERT_EQ(pHandle->clipPath(), clipPath.value<QPainterPath>());
    } else {
        ASSERT_TRUE(pHandle->clipPath().isEmpty());
    }

    QVariant frameMask = window->property(FRAMEMASK);

    if (frameMask.isValid() && frameMask.canConvert(QVariant::Region)) {
        ASSERT_EQ(pHandle->frameMask(), frameMask.value<QRegion>());
    } else {
        ASSERT_TRUE(pHandle->frameMask().isEmpty());
    }

    QVariant frameMargins = window->property(FRAMEMARGINS);

    if (frameMargins.isValid() && !frameMargins.value<QMargins>().isNull()) {
        ASSERT_EQ(pHandle->frameMargins(), frameMargins.value<QMargins>());
    } else {
        ASSERT_TRUE(pHandle->frameMargins().isNull());
    }

    QVariant translucentBackground = window->property(TRANSLUCENTBACKGROUND);
    if (translucentBackground.isValid() && translucentBackground.canConvert(QVariant::Bool)) {
        ASSERT_EQ(pHandle->translucentBackground(), translucentBackground.toBool());
    } else {
        ASSERT_FALSE(pHandle->translucentBackground());
    }

    QVariant enableSystemResize = window->property(ENABLESYSTEMRESIZE);
    if (enableSystemResize.isValid() && enableSystemResize.canConvert(QVariant::Bool)) {
        ASSERT_EQ(pHandle->enableSystemResize(), enableSystemResize.toBool());
    } else {
        ASSERT_FALSE(pHandle->enableSystemResize());
    }

    QVariant enableSystemMove = window->property(ENABLESYSTEMMOVE);
    if (enableSystemMove.isValid() && enableSystemMove.canConvert(QVariant::Bool)) {
        ASSERT_EQ(pHandle->enableSystemMove(), enableSystemMove.toBool());
    } else {
        ASSERT_FALSE(pHandle->enableSystemMove());
    }

    QVariant enableBlurWindow = window->property(ENABLEBLURWINDOW);
    if (enableBlurWindow.isValid() && enableBlurWindow.canConvert(QVariant::Bool)) {
        ASSERT_EQ(pHandle->enableBlurWindow(), enableBlurWindow.toBool());
    } else {
        ASSERT_FALSE(pHandle->enableBlurWindow());
    }

    QVariant autoInputMaskByClipPath = window->property(AUTOINPUTMASKBYCLIPPATH);
    if (autoInputMaskByClipPath.isValid() && autoInputMaskByClipPath.canConvert(QVariant::Bool)) {
        ASSERT_EQ(pHandle->autoInputMaskByClipPath(), autoInputMaskByClipPath.toBool());
    } else {
        ASSERT_FALSE(pHandle->autoInputMaskByClipPath());
    }

    QVariant realWindowId = window->property(REALWINDOWID);
    if (enableBlurWindow.isValid() && enableBlurWindow.value<WId>() != 0) {
        ASSERT_EQ(pHandle->realWindowId(), enableBlurWindow.value<WId>());
    } else {
        ASSERT_FALSE(pHandle->realWindowId());
    }
}

TEST_F(TDPlatformHandle, testSlots)
{
    enum { TESTBORDERWIDTH = 4, TESTOFFSET = 6, TESTRADIUS = 8 };
    if (pHandle) {
        pHandle->setWindowRadius(TESTRADIUS);
        ASSERT_EQ(pHandle->windowRadius(), TESTRADIUS);

        pHandle->setBorderWidth(TESTBORDERWIDTH);
        ASSERT_EQ(pHandle->borderWidth(), TESTBORDERWIDTH);

        pHandle->setBorderColor(Qt::black);
        ASSERT_EQ(pHandle->borderColor(), Qt::black);

        pHandle->setShadowRadius(TESTRADIUS);
        ASSERT_EQ(pHandle->shadowRadius(), TESTRADIUS);

        pHandle->setShadowOffset({TESTOFFSET, TESTOFFSET});
        ASSERT_EQ(pHandle->shadowOffset(), QPoint(TESTOFFSET, TESTOFFSET));

        pHandle->setShadowColor(Qt::blue);
        ASSERT_EQ(pHandle->shadowColor(), Qt::blue);

        QPainterPath pPath;
        pPath.addRect({0, 0, 20, 20});

        pHandle->setClipPath(pPath);
        ASSERT_EQ(pHandle->clipPath(), pPath);

        pHandle->setFrameMask(QRegion(0, 0, 10, 10));
        ASSERT_EQ(pHandle->frameMask(), QRegion(0, 0, 10, 10));

        pHandle->setTranslucentBackground(true);
        ASSERT_TRUE(pHandle->translucentBackground());

        pHandle->setEnableSystemResize(true);
        ASSERT_TRUE(pHandle->enableSystemResize());

        pHandle->setEnableSystemMove(true);
        ASSERT_TRUE(pHandle->enableSystemMove());

        pHandle->setEnableBlurWindow(true);
        ASSERT_TRUE(pHandle->enableBlurWindow());

        pHandle->setAutoInputMaskByClipPath(true);
        ASSERT_TRUE(pHandle->autoInputMaskByClipPath());
    }
}

TEST_F(TDPlatformHandle, wmAreaDebug)
{
    DPlatformHandle::WMBlurArea area = dMakeWMBlurArea(0, 0, 20, 20);

    QByteArray data;
    QBuffer buf(&data);
    ASSERT_TRUE(buf.open(QIODevice::WriteOnly));

    QDebug tDebug(&buf);
    tDebug << area;
    buf.close();
    ASSERT_FALSE(data.isEmpty());
}
