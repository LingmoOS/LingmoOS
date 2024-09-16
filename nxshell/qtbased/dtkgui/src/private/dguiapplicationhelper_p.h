// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGUIAPPLICATIONHELPER_P_H
#define DGUIAPPLICATIONHELPER_P_H

#include "dguiapplicationhelper.h"
#include "dplatformtheme.h"

#include <DObjectPrivate>

QT_BEGIN_NAMESPACE
class QLocalServer;
QT_END_NAMESPACE

DGUI_BEGIN_NAMESPACE

/*!
 @private
 */
class DGuiApplicationHelperPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
public:
    D_DECLARE_PUBLIC(DGuiApplicationHelper)

    DGuiApplicationHelperPrivate(DGuiApplicationHelper *qq);
    void init();
    void initApplication(QGuiApplication *app);
    static void staticInitApplication();
    static void staticCleanApplication();
    DPlatformTheme *initWindow(QWindow *window) const;
    void _q_initApplicationTheme(bool notifyChange = false);
    void _q_sizeModeChanged(int mode);
    DGuiApplicationHelper::SizeMode fetchSizeMode(bool *isSystemSizeMode = nullptr) const;
    void notifyAppThemeChanged();
    void notifyAppThemeChangedByEvent();
    void onApplicationPaletteChanged();
    // 返回程序是否自定义了调色板
    inline bool isCustomPalette() const;
    void setPaletteType(DGuiApplicationHelper::ColorType ct, bool emitSignal);
    void initPaletteType() const;

    DGuiApplicationHelper::ColorType paletteType = DGuiApplicationHelper::UnknownType;
    // 系统级别的主题设置
    DPlatformTheme *systemTheme = nullptr;
    QScopedPointer<DPalette> appPalette;
    // 获取QLocalSever消息的等待时间
    static int waitTime;
    static DGuiApplicationHelper::Attributes attributes;
    DGuiApplicationHelper::SizeMode systemSizeMode = DGuiApplicationHelper::NormalMode;
    DGuiApplicationHelper::SizeMode explicitSizeMode;

private:
    // 应用程序级别的主题设置
    DPlatformTheme *appTheme = nullptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DGuiApplicationHelper::Attributes)

DGUI_END_NAMESPACE

#endif // DGUIAPPLICATIONHELPER_P_H
