// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dplatformnativeinterfacehook.h"
#include "global.h"
#include "utility.h"
#include "dplatformwindowhelper.h"
#include "dplatformintegration.h"

#include "dwmsupport.h"

#ifdef Q_OS_LINUX
#include "xcbnativeeventfilter.h"
#include "qxcbnativeinterface.h"
typedef QXcbNativeInterface DPlatformNativeInterface;
#elif defined(Q_OS_WIN)
#include "qwindowsgdinativeinterface.h"
typedef QWindowsGdiNativeInterface DPlatformNativeInterface;
#endif

DPP_BEGIN_NAMESPACE

static QString version()
{
    return QStringLiteral(DXCB_VERSION);
}

#ifdef Q_OS_LINUX
static DeviceType _inputEventSourceDevice(const QInputEvent *event)
{
    return DPlatformIntegration::instance()->eventFilter()->xiEventSource(event);
}
#endif

static QFunctionPointer getFunction(const QByteArray &function)
{
    static QHash<QByteArray, QFunctionPointer> functionCache = {
        {setWmBlurWindowBackgroundArea, reinterpret_cast<QFunctionPointer>(&Utility::blurWindowBackground)},
        {setWmBlurWindowBackgroundPathList, reinterpret_cast<QFunctionPointer>(&Utility::blurWindowBackgroundByPaths)},
        {setWmBlurWindowBackgroundMaskImage, reinterpret_cast<QFunctionPointer>(&Utility::blurWindowBackgroundByImage)},
        {setWmWallpaperParameter, reinterpret_cast<QFunctionPointer>(&Utility::updateBackgroundWallpaper)},
        {hasBlurWindow, reinterpret_cast<QFunctionPointer>(&DWMSupport::Global::hasBlurWindow)},
        {hasComposite, reinterpret_cast<QFunctionPointer>(&DWMSupport::Global::hasComposite)},
        {hasNoTitlebar, reinterpret_cast<QFunctionPointer>(&DWMSupport::Global::hasNoTitlebar)},
        {hasWindowAlpha, reinterpret_cast<QFunctionPointer>(&DWMSupport::Global::hasWindowAlpha)},
        {hasWallpaperEffect, reinterpret_cast<QFunctionPointer>(&DWMSupport::Global::hasWallpaperEffect)},
        {windowManagerName, reinterpret_cast<QFunctionPointer>(&DWMSupport::Global::windowManagerName)},
        {connectWindowManagerChangedSignal, reinterpret_cast<QFunctionPointer>(&DWMSupport::connectWindowManagerChangedSignal)},
        {connectHasBlurWindowChanged, reinterpret_cast<QFunctionPointer>(&DWMSupport::connectHasBlurWindowChanged)},
        {connectHasCompositeChanged, reinterpret_cast<QFunctionPointer>(&DWMSupport::connectHasCompositeChanged)},
        {connectHasNoTitlebarChanged, reinterpret_cast<QFunctionPointer>(&DWMSupport::connectHasNoTitlebarChanged)},
        {connectHasWallpaperEffectChanged, reinterpret_cast<QFunctionPointer>(&DWMSupport::connectHasWallpaperEffectChanged)},
        {getWindows, reinterpret_cast<QFunctionPointer>(&Utility::getWindows)},
        {windowFromPoint, reinterpret_cast<QFunctionPointer>(&Utility::windowFromPoint)},
        {getCurrentWorkspaceWindows, reinterpret_cast<QFunctionPointer>(&Utility::getCurrentWorkspaceWindows)},
        {connectWindowListChanged, reinterpret_cast<QFunctionPointer>(&DWMSupport::connectWindowListChanged)},
        {setMWMFunctions, reinterpret_cast<QFunctionPointer>(&DWMSupport::setMWMFunctions)},
        {getMWMFunctions, reinterpret_cast<QFunctionPointer>(&DWMSupport::getMWMFunctions)},
        {setMWMDecorations, reinterpret_cast<QFunctionPointer>(&DWMSupport::setMWMDecorations)},
        {getMWMDecorations, reinterpret_cast<QFunctionPointer>(&DWMSupport::getMWMDecorations)},
        {connectWindowMotifWMHintsChanged, reinterpret_cast<QFunctionPointer>(&DWMSupport::connectWindowMotifWMHintsChanged)},
        {popupSystemWindowMenu, reinterpret_cast<QFunctionPointer>(&DWMSupport::popupSystemWindowMenu)},
        {setWindowProperty, reinterpret_cast<QFunctionPointer>(&DPlatformIntegration::setWindowProperty)},
        {pluginVersion, reinterpret_cast<QFunctionPointer>(&version)},
        {inputEventSourceDevice, reinterpret_cast<QFunctionPointer>(&_inputEventSourceDevice)},
        {createGroupWindow, reinterpret_cast<QFunctionPointer>(&Utility::createGroupWindow)},
        {destoryGroupWindow, reinterpret_cast<QFunctionPointer>(&Utility::destoryGroupWindow)},
        {setWindowGroup, reinterpret_cast<QFunctionPointer>(&Utility::setWindowGroup)},
        {clientLeader, reinterpret_cast<QFunctionPointer>(&Utility::clientLeader)},
        {enableDxcb, reinterpret_cast<QFunctionPointer>(&DPlatformIntegration::enableDxcb)},
        {isEnableDxcb, reinterpret_cast<QFunctionPointer>(&DPlatformIntegration::isEnableDxcb)},
        {setEnableNoTitlebar, reinterpret_cast<QFunctionPointer>(&DPlatformIntegration::setEnableNoTitlebar)},
        {isEnableNoTitlebar, reinterpret_cast<QFunctionPointer>(&DPlatformIntegration::isEnableNoTitlebar)},
        {buildNativeSettings, reinterpret_cast<QFunctionPointer>(&DPlatformIntegration::buildNativeSettings)},
        {clearNativeSettings, reinterpret_cast<QFunctionPointer>(&DPlatformIntegration::clearNativeSettings)},
        {setWMClassName, reinterpret_cast<QFunctionPointer>(&DPlatformIntegration::setWMClassName)},
        {splitWindowOnScreen, reinterpret_cast<QFunctionPointer>(&Utility::splitWindowOnScreen)},
        {supportForSplittingWindow, reinterpret_cast<QFunctionPointer>(&Utility::supportForSplittingWindow)},
        {sendEndStartupNotifition, reinterpret_cast<QFunctionPointer>(&DPlatformIntegration::sendEndStartupNotifition)},
        {splitWindowOnScreenByType, reinterpret_cast<QFunctionPointer>(&Utility::splitWindowOnScreenByType)},
        {supportForSplittingWindowByType, reinterpret_cast<QFunctionPointer>(&Utility::supportForSplittingWindowByType)}
    };

    return functionCache.value(function);
}

QFunctionPointer DPlatformNativeInterfaceHook::platformFunction(QPlatformNativeInterface *interface, const QByteArray &function)
{
    QFunctionPointer f = getFunction(function);

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
        if (!f) {
            f = static_cast<DPlatformNativeInterface*>(interface)->DPlatformNativeInterface::platformFunction(function);
        }
#endif
    return f;
}

DPP_END_NAMESPACE
