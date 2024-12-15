// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globalstatus.h"

static const int sc_MinHeight = 300;           // 窗口最小高度
static const int sc_MinWidth = 628;            // 窗口最小宽度
static const int sc_MinHideHeight = 425;       // 调整窗口高度小于425px时，隐藏工具栏和标题栏
static const int sc_FloatMargin = 65;          // 浮动按钮边距
static const int sc_TitleHeight = 50;          // 标题栏栏高度
static const int sc_ThumbnailViewHeight = 70;  // 底部工具栏高度 70px
static const int sc_ShowBottomY = 80;  // 底部工具栏显示时距离底部的高度 80px (工具栏高度 70px + 边距 10px)
static const int sc_SwitchImageHotspotWidth = 100;  // 左右切换图片按钮的热区宽度 100px
static const int sc_ActionMargin = 9;               // 应用图标距离顶栏
static const int sc_RightMenuItemHeight = 32;       // 右键菜单item的高度

static const double sc_AnimationDefaultDuration = 366;  // 默认动画时长
static const int sc_PathViewItemCount = 3;              // 默认 PathView 在路径中的 Item 计数

/**
   @class GlobalStatus
   @brief QML单例类，维护全局状态，同步不同组件间的状态信息
   @details 相较于使用脚本配置的 program Singletion , Qt 更推崇使用 QObject 注册单例
   @link https://doc.qt.io/qt-6/qtquick-performance.html#use-singleton-types-instead-of-pragma-library-scripts
 */

GlobalStatus::GlobalStatus(QObject *parent)
    : QObject(parent)
{
}

GlobalStatus::~GlobalStatus() { }

/**
   @return 返回是否全屏显示图片
 */
bool GlobalStatus::showFullScreen() const
{
    return storeshowFullScreen;
}

/**
   @brief 设置全屏显示图片
 */
void GlobalStatus::setShowFullScreen(bool value)
{
    if (value != storeshowFullScreen) {
        storeshowFullScreen = value;
        Q_EMIT showFullScreenChanged();
    }
}

/**
   @return 返回是否允许显示导航窗口
 */
bool GlobalStatus::enableNavigation() const
{
    return storeenableNavigation;
}

/**
   @brief 设置是否允许显示导航窗口
 */
void GlobalStatus::setEnableNavigation(bool value)
{
    if (value != storeenableNavigation) {
        storeenableNavigation = value;
        Q_EMIT enableNavigationChanged();
    }
}

/**
   @return 返回是否显示右键菜单
 */
bool GlobalStatus::showRightMenu() const
{
    return storeshowRightMenu;
}

/**
   @brief 设置是否显示右键菜单
 */
void GlobalStatus::setShowRightMenu(bool value)
{
    if (value != storeshowRightMenu) {
        storeshowRightMenu = value;
        Q_EMIT showRightMenuChanged();
    }
}

/**
   @return 当前是否弹窗显示详细图像信息
 */
bool GlobalStatus::showImageInfo() const
{
    return storeshowImageInfo;
}

/**
   @brief 设置是否弹窗显示详细图像信息
 */
void GlobalStatus::setShowImageInfo(bool value)
{
    if (value != storeshowImageInfo) {
        storeshowImageInfo = value;
        Q_EMIT showImageInfoChanged();
    }
}

/**
   @return 返回滑动视图是否响应操作
 */
bool GlobalStatus::viewInteractive() const
{
    return storeviewInteractive;
}

/**
   @brief 设置滑动视图是否响应操作
 */
void GlobalStatus::setViewInteractive(bool value)
{
    if (value != storeviewInteractive) {
        storeviewInteractive = value;
        Q_EMIT viewInteractiveChanged();
    }
}

/**
   @return 返回滑动视图是否处于轻弹状态
 */
bool GlobalStatus::viewFlicking() const
{
    return storeviewFlicking;
}

/**
   @brief 设置当前滑动视图是否处于轻弹状态
 */
void GlobalStatus::setViewFlicking(bool value)
{
    if (value != storeviewFlicking) {
        storeviewFlicking = value;
        Q_EMIT viewFlickingChanged();
    }
}

/**
   @return 返回当前是否允许标题栏、底栏动画效果
 */
bool GlobalStatus::animationBlock() const
{
    return storeanimationBlock;
}

/**
   @brief 设置当前允许标题栏、底栏动画效果的标志值为 \a value
 */
void GlobalStatus::setAnimationBlock(bool value)
{
    if (value != storeanimationBlock) {
        storeanimationBlock = value;
        Q_EMIT animationBlockChanged();
    }
}

/**
   @return 返回当前是否允许全屏展示动画
 */
bool GlobalStatus::fullScreenAnimating() const
{
    return storefullScreenAnimating;
}

/**
   @brief 设置当前是否允许全屏展示动画的标志值为 \a value
 */
void GlobalStatus::setFullScreenAnimating(bool value)
{
    if (value != storefullScreenAnimating) {
        storefullScreenAnimating = value;
        Q_EMIT fullScreenAnimatingChanged();
    }
}

/**
   @return 返回当前缩略图列表允许显示的宽度
 */
int GlobalStatus::thumbnailVaildWidth() const
{
    return storethumbnailVaildWidth;
}

/**
   @brief 设置当前缩略图列表允许显示的宽度为 \a value
 */
void GlobalStatus::setThumbnailVaildWidth(int value)
{
    if (value != storethumbnailVaildWidth) {
        storethumbnailVaildWidth = value;
        Q_EMIT thumbnailVaildWidthChanged();
    }
}

/**
   @return 返回当前显示的界面索引
 */
Types::StackPage GlobalStatus::stackPage() const
{
    return storestackPage;
}

/**
   @brief 设置当前显示的界面索引为 \a value ，将切换显示的界面类型
 */
void GlobalStatus::setStackPage(Types::StackPage value)
{
    if (value != storestackPage) {
        storestackPage = value;
        Q_EMIT stackPageChanged();
    }
}

int GlobalStatus::minHeight() const
{
    return sc_MinHeight;
}

int GlobalStatus::minWidth() const
{
    return sc_MinWidth;
}

int GlobalStatus::minHideHeight() const
{
    return sc_MinHideHeight;
}

int GlobalStatus::floatMargin() const
{
    return sc_FloatMargin;
}

int GlobalStatus::titleHeight() const
{
    return sc_TitleHeight;
}

int GlobalStatus::thumbnailViewHeight() const
{
    return sc_ThumbnailViewHeight;
}

int GlobalStatus::showBottomY() const
{
    return sc_ShowBottomY;
}

int GlobalStatus::switchImageHotspotWidth() const
{
    return sc_SwitchImageHotspotWidth;
}

int GlobalStatus::actionMargin() const
{
    return sc_ActionMargin;
}

int GlobalStatus::rightMenuItemHeight() const
{
    return sc_RightMenuItemHeight;
}

double GlobalStatus::animationDefaultDuration() const
{
    return sc_AnimationDefaultDuration;
}

/**
   @brief 默认 PathView 在路径中的 Item 计数
   @note 会影响 PathView 相关的动画效果计算，修改此值需慎重考虑
 */
int GlobalStatus::pathViewItemCount() const
{
    return sc_PathViewItemCount;
}
