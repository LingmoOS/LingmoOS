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

#ifndef KMENUBUTTON_H
#define KMENUBUTTON_H

#include "gui_g.h"
#include "themeController.h"
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QToolButton>

namespace kdk
{

/**
 * @defgroup PushbuttonModule
 * @{
 */

class KMenuButtonPrivate;

/**
 * @brief 下拉菜单按钮
 */
class GUI_EXPORT KMenuButton:public QToolButton,public ThemeController
{
    Q_OBJECT
public:
    KMenuButton(QWidget*parent=nullptr);
    ~KMenuButton();

    /**
     * @brief 获取主菜单
     * @return
     */
    QMenu* menu();

    /**
     * @brief 获取主题菜单
     * @return
     */
    QMenu* themeMenu();

    /**
     * @brief 获取设置action
     * @return
     */
    QAction* settingAction();

    /**
     * @brief 获取主题Action
     * @return
     */
    QAction* themeAction();

    /**
     * @brief 获取帮助Action
     * @return
     */
    QAction* assistAction();

    /**
     * @brief 获取关于Action
     * @return
     */
    QAction* aboutAction();

    /**
     * @brief 获取意见反馈Action
     * @since 2.4.1.0-0k1.0
     * @return
     */
    QAction* feedbackAction();
    /**
     * @brief 获取离开Action
     * @return
     */
    QAction* quitAction();

    /**
     * @brief 获取跟随主题Action
     * @return
     */
    QAction* autoAction();

    /**
     * @brief 获取浅色主题Action
     * @return
     */
    QAction* lightAction();

    /**
     * @brief 获取深色Action
     * @return
     */
    QAction* darkAction();

protected:
    void changeTheme() override;
    void paintEvent(QPaintEvent*painteEvent);
    QSize sizeHint() const override;

private:
    Q_DECLARE_PRIVATE(KMenuButton)
    KMenuButtonPrivate* const d_ptr;
};
}
/**
  * @example testWidget/testwidget.h
  * @example testWidget/testwidget.cpp
  * @}
  */
#endif // KMENUBUTTON_H
