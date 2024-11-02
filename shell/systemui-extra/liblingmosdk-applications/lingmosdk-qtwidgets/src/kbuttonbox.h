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

#ifndef KBUTTONBOX_H
#define KBUTTONBOX_H

#include "gui_g.h"
#include "kpushbutton.h"
#include <QWidget>
#include <QAbstractButton>


namespace kdk {

/**
 *  @defgroup ContainerModules
 */
class KButtonBoxPrivate;

/**
 * @brief 提供了一个按钮类容器
 * @since 2.0
 */
class GUI_EXPORT KButtonBox : public QWidget
{
    Q_OBJECT

public:
    explicit KButtonBox(QWidget *parent = nullptr);
    ~KButtonBox();

    /**
     * @brief 获取KButtonBox的布局类型
     * @return
     */
    Qt::Orientation orientation();

    /**
     * @brief 设置KButtonBox的布局类型,包括水平类型和垂直类型
     * @param orientation
     */
    void setOrientation(Qt::Orientation orientation);

    /**
     * @brief 添加按钮
     * @param button
     */
    void addButton(KPushButton *button,int i = -1);

    /**
     * @brief 删除按钮
     * @param button
     */
    void removeButton(KPushButton *button);

    /**
     * @brief 按id删除指定按钮
     * @param i
     */
    void removeButton(int i);

    /**
     * @brief 以列表形式向KButtonBox中添加按钮
     * @param list
     * @param checkable
     */
    void setButtonList(const QList<KPushButton*> &list);

    /**
     * @brief 获取KButtonBox中的按钮列表
     * @return
     */
    QList<KPushButton *> buttonList();

    /**
     * @brief 设置KButtonBox首尾部分按钮的圆角
     * @param radius
     */
    void setBorderRadius(int radius);

    /**
     * @brief 获取KButtonBox首尾部分按钮的圆角
     * @return
     */
    int borderRadius();

    /**
     * @brief 设置按钮id
     * @param button
     * @param id
     */
    void setId(KPushButton *button, int id);

    /**
     * @brief 获取按钮id
     * @param button
     * @return
     */
    int id(KPushButton *button);

    /**
     * @brief 返回已选中的按钮
     * @return
     */
    KPushButton * checkedButton();

    /**
     * @brief 通过按钮id获取按钮
     * @param id
     * @return
     */
    KPushButton *button(int id);

    /**
     * @brief 返回已选中按钮的id
     * @return
     */
    int checkedId();

    /**
     * @brief 设置KButtonBox按钮间是否互斥
     */
    void setExclusive(bool);

    /**
     * @brief 返回KButtonBox按钮间是否互斥
     * @return
     */
    bool exclusive();

    /**
     * @brief 设置KButtonBox中的按钮是否可选中
     */
    void setCheckable(bool flag);

    /**
     * @brief 返回KButtonBox中的按钮是否可选中
     * @return
     */
    bool isCheckable();

Q_SIGNALS:
    void buttonClicked(QAbstractButton *);
    void buttonPressed(QAbstractButton *);
    void buttonReleased(QAbstractButton *);
    void buttonToggled(QAbstractButton *, bool);

private:
    Q_DECLARE_PRIVATE(KButtonBox)
    KButtonBoxPrivate* const d_ptr;
};

}
#endif // KBUTTONBOX_H
