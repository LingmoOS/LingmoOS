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

#ifndef KUNINSTALLDIALOG_H
#define KUNINSTALLDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include "gui_g.h"
#include "themeController.h"
#include "kdialog.h"

namespace kdk
{

/** @defgroup DialogBoxModule
  * @{
  */

static const QByteArray ORG_LINGMO_STYLE_FONT = "org.lingmo.style";
static const QByteArray GSETTING_FONT_KEY = "systemFontSize";

class KUninstallDialogPrivate;

/**
 * @brief 卸载对话框，当用户卸载时可能会弹出此提示对话框
 */
class GUI_EXPORT KUninstallDialog:public KDialog
{
    Q_OBJECT

public:
    KUninstallDialog(QString appName, QString appVersion, QWidget *parent=nullptr);
    ~KUninstallDialog();

    /**
     * @brief debAppNameLabel
     * @return
     */
    QLabel* debAppNameLabel();

    /**
     * @brief 包名Label
     * @return
     */
    QLabel* debNameLabel();

    /**
     * @brief 包IconLabel
     * @return
     */
    QLabel* debIconLabel();

    /**
     * @brief 包版本Label
     * @return
     */
    QLabel* debVersionLabel();

    /**
     * @brief 卸载按钮
     * @return
     */
    QPushButton* uninstallButtton();

protected:
    void changeTheme();

private:
    Q_DECLARE_PRIVATE(KUninstallDialog)
    KUninstallDialogPrivate*const d_ptr;
};
}
/**
  * @example testDialog/widget.h
  * @example testDialog/widget.cpp
  * @}
  */
#endif // KUNINSTALLDIALOG_H
