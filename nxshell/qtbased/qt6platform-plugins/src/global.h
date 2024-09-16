// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef GLOBAL_H
#define GLOBAL_H

#include <qwindowdefs.h>
#include <QObject>

#define MOUSE_MARGINS 10

#define DPP_BEGIN_NAMESPACE namespace deepin_platform_plugin {
#define DPP_END_NAMESPACE }
#define DPP_USE_NAMESPACE using namespace deepin_platform_plugin;

#define PUBLIC_CLASS(Class, Target) \
    class D##Class : public Class\
    {friend class Target;}

#define DEFINE_CONST_CHAR(Name) const char Name[] = "_d_" #Name

DEFINE_CONST_CHAR(useDxcb);
DEFINE_CONST_CHAR(useDwayland);
DEFINE_CONST_CHAR(useNativeDxcb);
DEFINE_CONST_CHAR(redirectContent);
DEFINE_CONST_CHAR(netWmStates);
DEFINE_CONST_CHAR(windowRadius);
DEFINE_CONST_CHAR(borderWidth);
DEFINE_CONST_CHAR(borderColor);
DEFINE_CONST_CHAR(shadowRadius);
DEFINE_CONST_CHAR(shadowOffset);
DEFINE_CONST_CHAR(shadowColor);
DEFINE_CONST_CHAR(windowStartUpEffect);
DEFINE_CONST_CHAR(clipPath);
DEFINE_CONST_CHAR(frameMask);
DEFINE_CONST_CHAR(frameMargins);
DEFINE_CONST_CHAR(translucentBackground);
DEFINE_CONST_CHAR(enableCloseable);
DEFINE_CONST_CHAR(windowEffect);
DEFINE_CONST_CHAR(enableSystemResize);
DEFINE_CONST_CHAR(enableSystemMove);
DEFINE_CONST_CHAR(enableBlurWindow);
DEFINE_CONST_CHAR(userWindowMinimumSize);
DEFINE_CONST_CHAR(userWindowMaximumSize);
DEFINE_CONST_CHAR(windowBlurAreas);
DEFINE_CONST_CHAR(windowBlurPaths);
DEFINE_CONST_CHAR(autoInputMaskByClipPath);
DEFINE_CONST_CHAR(popupSystemWindowMenu);
DEFINE_CONST_CHAR(groupLeader);
DEFINE_CONST_CHAR(noTitlebar);
DEFINE_CONST_CHAR(enableGLPaint);
DEFINE_CONST_CHAR(windowInWorkSpace);

// functions
DEFINE_CONST_CHAR(setWmBlurWindowBackgroundArea);
DEFINE_CONST_CHAR(setWmBlurWindowBackgroundPathList);
DEFINE_CONST_CHAR(setWmBlurWindowBackgroundMaskImage);
DEFINE_CONST_CHAR(setWmWallpaperParameter);
DEFINE_CONST_CHAR(hasBlurWindow);
DEFINE_CONST_CHAR(hasComposite);
DEFINE_CONST_CHAR(hasNoTitlebar);
DEFINE_CONST_CHAR(hasWindowAlpha);
DEFINE_CONST_CHAR(hasWallpaperEffect);
DEFINE_CONST_CHAR(windowManagerName);
DEFINE_CONST_CHAR(connectWindowManagerChangedSignal);
DEFINE_CONST_CHAR(connectHasBlurWindowChanged);
DEFINE_CONST_CHAR(connectHasCompositeChanged);
DEFINE_CONST_CHAR(connectHasNoTitlebarChanged);
DEFINE_CONST_CHAR(connectHasWallpaperEffectChanged);
DEFINE_CONST_CHAR(getWindows);
DEFINE_CONST_CHAR(windowFromPoint);
DEFINE_CONST_CHAR(getCurrentWorkspaceWindows);
DEFINE_CONST_CHAR(connectWindowListChanged);
DEFINE_CONST_CHAR(setMWMFunctions);
DEFINE_CONST_CHAR(getMWMFunctions);
DEFINE_CONST_CHAR(setMWMDecorations);
DEFINE_CONST_CHAR(getMWMDecorations);
DEFINE_CONST_CHAR(connectWindowMotifWMHintsChanged);
DEFINE_CONST_CHAR(setWindowProperty);
DEFINE_CONST_CHAR(pluginVersion);
DEFINE_CONST_CHAR(disableOverrideCursor);
DEFINE_CONST_CHAR(inputEventSourceDevice);
DEFINE_CONST_CHAR(createGroupWindow);
DEFINE_CONST_CHAR(destoryGroupWindow);
DEFINE_CONST_CHAR(setWindowGroup);
DEFINE_CONST_CHAR(clientLeader);
DEFINE_CONST_CHAR(enableDxcb);
DEFINE_CONST_CHAR(isEnableDxcb);
DEFINE_CONST_CHAR(enableDwayland);
DEFINE_CONST_CHAR(isEnableDwayland);
DEFINE_CONST_CHAR(setEnableNoTitlebar);
DEFINE_CONST_CHAR(setWindowRadius);
DEFINE_CONST_CHAR(setBorderColor);
DEFINE_CONST_CHAR(setShadowColor);
DEFINE_CONST_CHAR(setShadowRadius);
DEFINE_CONST_CHAR(setShadowOffset);
DEFINE_CONST_CHAR(setBorderWidth);
DEFINE_CONST_CHAR(setWindowEffect);
DEFINE_CONST_CHAR(setWindowStartUpEffect);
DEFINE_CONST_CHAR(isEnableNoTitlebar);
DEFINE_CONST_CHAR(buildNativeSettings);
DEFINE_CONST_CHAR(clearNativeSettings);
DEFINE_CONST_CHAR(setWMClassName);
DEFINE_CONST_CHAR(splitWindowOnScreen);
DEFINE_CONST_CHAR(supportForSplittingWindow);
DEFINE_CONST_CHAR(sendEndStartupNotifition);
DEFINE_CONST_CHAR(splitWindowOnScreenByType);
DEFINE_CONST_CHAR(supportForSplittingWindowByType);

// others
DEFINE_CONST_CHAR(WmWindowTypes);
DEFINE_CONST_CHAR(WmNetDesktop);
DEFINE_CONST_CHAR(WmClass);
DEFINE_CONST_CHAR(ProcessId);

enum DeviceType {
    UnknowDevice,
    TouchapdDevice,
    MouseDevice
};

class QWindow;
QWindow * fromQtWinId(WId id);

DPP_BEGIN_NAMESPACE

class RunInThreadProxy : public QObject
{
    Q_OBJECT
public:
    using FunctionType = std::function<void()>;
    explicit RunInThreadProxy(QObject *parent = nullptr);
    void proxyCall(FunctionType func);

};
DPP_END_NAMESPACE
#endif // GLOBAL_H
