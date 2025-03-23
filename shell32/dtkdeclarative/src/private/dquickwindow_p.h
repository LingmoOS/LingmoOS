// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWINDOW_P_H
#define DWINDOW_P_H

#include <DObjectPrivate>
#include <DObject>
#include <DPlatformHandle>

#include "dquickwindow.h"

DQUICK_BEGIN_NAMESPACE

class DQuickBehindWindowBlur;
class DQuickWindowPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    explicit DQuickWindowPrivate(DQuickWindow *qq);

    DQuickWindowAttached *attached;

private:
    D_DECLARE_PUBLIC(DQuickWindow)
};

class DQuickWindowAttachedPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    enum BoolOptional : qint8 {
        Invalid = -1,
        False = 0,
        True = 1
    };

    explicit DQuickWindowAttachedPrivate(QWindow *window, DQuickWindowAttached *qq);
    ~DQuickWindowAttachedPrivate() override;

    bool ensurePlatformHandle();
    void destoryPlatformHandle();
    void _q_onWindowMotifHintsChanged(quint32 winId);
    void addBlur(DQuickBehindWindowBlur *blur);
    void removeBlur(DQuickBehindWindowBlur *blur);
    void updateBlurAreaFor(DQuickBehindWindowBlur *blur);
    void _q_updateBlurAreaForWindow();
    void _q_updateClipPath();
    void _q_ensurePlatformHandle();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void ensurePalette();
    void _q_updateWindowPalette();
    void _q_onPaletteChanged();
#endif

    QWindow *window = nullptr;
    DPlatformHandle *handle = nullptr;
    BoolOptional explicitEnable {Invalid};
    BoolOptional explicitTranslucentBackground {Invalid};
    BoolOptional explicitEnableSystemResize {Invalid};
    BoolOptional explicitEnableSystemMove {Invalid};
    BoolOptional explicitEnableBlurWindow {Invalid};
    qint8 explicitWindowRadius = -1;
    qint8 explicitBorderWidth = -1;
    qint8 explicitShadowRadius = -1;
    QPoint explicitShadowOffset;
    QColor explicitBorderColor;
    QColor explicitShadowColor;
    DPlatformHandle::EffectScenes explicitEffectScene = DPlatformHandle::EffectScenes(0);
    DPlatformHandle::EffectTypes explicitEffectType = DPlatformHandle::EffectTypes(0);

    DWindowManagerHelper::WmWindowTypes wmWindowTypes;
    DWindowManagerHelper::MotifFunctions motifFunctions;
    DWindowManagerHelper::MotifDecorations motifDecorations;

    QList<DQuickBehindWindowBlur*> blurList;
    QQuickPath *clipPath = nullptr;
    QQuickTransition *overlayExitedTransition = nullptr;
    QQmlComponent *loadingOverlay = nullptr;
    DQuickAppLoaderItem *appLoaderItem;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    DGuiApplicationHelper::ColorType themeType{DGuiApplicationHelper::UnknownType};
    bool paletteInit = false;
    QQuickPalette *quickPalette = nullptr;
    QQuickPalette *inactiveQuickPalette = nullptr;
    std::array<QMetaObject::Connection, 2> paletteConnections;
#endif
private:
    D_DECLARE_PUBLIC(DQuickWindowAttached)
};

DQUICK_END_NAMESPACE

#endif //DWINDOW_P_H
