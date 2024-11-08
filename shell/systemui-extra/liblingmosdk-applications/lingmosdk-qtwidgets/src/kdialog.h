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

#ifndef KDIALOG_H
#define KDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QBoxLayout>
#include "gui_g.h"
#include "kiconbar.h"
#include "kwindowbuttonbar.h"
#include "themeController.h"


namespace kdk
{

/** @defgroup DialogBoxModule
  * @{
  */

class KDialogPrivate;
/**
 * @brief 继承自 QDialog，支持响应式主题背景切换和图标主题切换、 标题颜色响应窗口激活状态
 */
class GUI_EXPORT KDialog: public QDialog,public ThemeController
{
    Q_OBJECT

public:
    explicit KDialog(QWidget* parent=nullptr);
    ~KDialog();

    /**
     * @brief 设置对话框图标
     * @param icon
     */
    void setWindowIcon(const QIcon &icon);

    /**
     * @brief 设置对话框图标
     * @param iconName
     */
    void setWindowIcon(const QString& iconName);

    /**
     * @brief 设置对话框标题名称
     * @param widgetName
     */
    void setWindowTitle(const QString &);

    /**
     * @brief 获取最大化按钮
     * @return
     */
    QPushButton* maximumButton();

    /**
     * @brief 获取最小化按钮
     * @return
     */
    QPushButton* minimumButton();

    /**
     * @brief 获取关闭按钮
     * @return
     */
    QPushButton* closeButton();

    /**
     * @brief 获取下拉菜单按钮
     * @return
     */
    KMenuButton* menuButton();

    /**
     * @brief 获取主内容区Widget
     * @return
     */
    QWidget* mainWidget();

protected:
    bool eventFilter(QObject *target, QEvent *event) override;
    void changeTheme();
    void changeIconStyle();
    QBoxLayout* mainLayout();

private:
    Q_DECLARE_PRIVATE(KDialog)
    KDialogPrivate* const d_ptr;
};
}
/**
  * @example testDialog/widget.h
  * @example testDialog/widget.cpp
  * @}
  */

#endif // KDIALOG_H
