/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KWIDGET_H
#define KWIDGET_H

#include <QWidget>
#include <QIcon>
#include <QEvent>
#include "kwindowbuttonbar.h"
#include "kiconbar.h"

namespace kdk
{
/**
 * @brief 支持切换三种结构布局
 */
enum LayoutType
{
    VerticalType, //上下结构
    HorizontalType, //左右结构
    MixedType //过渡结构
};

/** @defgroup FormModule
  * @{
  */

class KWidgetPrivate;
/**
 * @brief 继承自QWidget,支持响应主题背景切换，图标主题切换，标题颜色响应窗口激活状态，内容区域分为side区和base区
 */
class GUI_EXPORT KWidget : public QWidget,public ThemeController
{
    Q_OBJECT

public:
    explicit KWidget(QWidget *parent = nullptr);
    ~KWidget();

    /**
     * @brief 设置窗体图标
     * @param icon
     */
    void setIcon(const QIcon& icon);

    /**
     * @brief 设置窗体图标
     * @param iconName 直接指定系统目录中的图标名称，如"lingmo-music"
     */
    void setIcon(const QString& iconName);

    /**
     * @brief 设置窗体名称
     * @param widgetName
     */
    void setWidgetName(const QString& widgetName);

    /**
     * @brief 获取左边栏widget,通过setlayout添加自定义内容
     * @return 返回左边栏widget
     */
    QWidget* sideBar();

    /**
     * @brief 获取主内容区widget,通过setlayout添加自定义内容
     * @return 返回主内容区widget
     */
    QWidget* baseBar();

    /**
     * @brief 获取窗口三联组合控件，以控制是否显示最大化、最小化按钮和下拉菜单按钮；也可增加自定义按钮
     * @return 返回窗口三联组合控件
     */
    KWindowButtonBar* windowButtonBar();

    /**
     * @brief 获取窗口标题、图标组合控件，以控制相关样式
     * @return 返回窗口标题、图标组合控件
     */
    KIconBar* iconBar();

    /**
     * @brief 设置布局结构类型
     */
    void setLayoutType(LayoutType type);

    /**
     * @brief 设置窗口标志
     * @param type
     * @since 1.2
     */
    void setWindowFlags(Qt::WindowFlags type);

    /**
     * @brief 设置窗口标志
     * @param flag
     * @param on
     * @since 1.2
     */
    void setWindowFlag(Qt::WindowType flag, bool on = true);

    /**
     * @brief 设置sidebar是否遵循系统更改宽度
     * @param flag
     * @since 1.2
     */
    void setSidebarFollowMode(bool flag);

    /**
     * @brief 返回sidebar是否遵循系统更改宽度
     * @return
     * @since 1.2
     */
    bool sidebarFollowMode();

protected:
    bool eventFilter(QObject *target, QEvent *event) override;
    virtual void changeIconStyle();
    virtual void changeTheme();

private:
    Q_DECLARE_PRIVATE(KWidget)
    KWidgetPrivate*const d_ptr;
};

}
/**
  * @example testWidget/testwidget.h
  * @example testWidget/testwidget.cpp
  * @}
  */
#endif // KWIDGET_H
