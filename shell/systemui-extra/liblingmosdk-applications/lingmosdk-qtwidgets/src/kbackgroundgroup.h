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

#ifndef KBACKGROUNDGROUP_H
#define KBACKGROUNDGROUP_H

#include "gui_g.h"
#include <QFrame>
#include <QWidget>
namespace kdk {

/**
 * @defgroup ContainerModules
 */
class KBackgroundGroupPrivate;

/**
 * @brief 提供了一个用于存储部件的容器
 * @since 2.0
 */

class GUI_EXPORT KBackgroundGroup : public QFrame
{
    Q_OBJECT

public:
    KBackgroundGroup(QWidget* parent = nullptr);

    /**
     * @brief 添加一个widget控件
     * @param widget
     */
    void addWidget(QWidget* widget);

    /**
     * @brief 添加一个widget list
     * @param list
     */
    void addWidgetList(QList<QWidget*> list);

    /**
     * @brief 删除一个指定index的widget
     * @param i
     */
    void removeWidgetAt(int i);

    /**
     * @brief 删除一个widget
     * @param widget
     */
    void removeWidget(QWidget* widget);

    /**
     * @brief 删除一个widget list
     * @param list
     */
    void removeWidgetList(QList<QWidget*> list);

    /**
     * @brief 指定位置插入一个widget
     * @param index
     * @param widget
     */
    void insertWidgetAt(int index, QWidget *widget);

    /**
     * @brief 指定位置插入一个widget list
     * @param index
     * @param list
     */
    void insertWidgetList(int index, QList<QWidget*> list);

    /**
     * @brief 设置KBackgroundGroup的圆角
     * @param radius
     */
    void setBorderRadius(int radius);

    /**
     * @brief 返回KBackgroundGroup的圆角
     * @return
     */
    int borderRadius();

    /**
     * @brief 设置背景颜色
     * @param role
     */
    void setBackgroundRole(QPalette::ColorRole role);

    /**
     * @brief 返回背景颜色
     * @return
     */
    QPalette::ColorRole backgroundRole() const;

    /**
     * @brief 设置窗体是否可以响应三态
     * @param flag
     * @param widget
     */
    void setStateEnable(QWidget* widget,bool flag);

    /**
     * @brief 返回widget列表
     * @return
     */
    QList<QWidget *> widgetList();

Q_SIGNALS:
    /**
     * @brief 点击会发出信号
     */
    void clicked(QWidget*);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Q_DECLARE_PRIVATE(KBackgroundGroup)
    KBackgroundGroupPrivate* const d_ptr;
};

}
#endif // KBACKGROUNDGROUP_H
