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

#ifndef KABOUTDIALOG_H
#define KABOUTDIALOG_H

#include <QDialog>
#include <QString>
#include <QIcon>
#include "gui_g.h"
#include "themeController.h"
#include "kdialog.h"

namespace kdk
{
/** @defgroup DialogBoxModule
  * @{
  */
class KAboutDialogPrivate;

/**
 * @brief 关于对话框，支持两种样式，一种是无特定说明的，另一种是有特定说明的。
 */
class GUI_EXPORT KAboutDialog:public KDialog
{
    Q_OBJECT

public:
    KAboutDialog(QWidget*parent = nullptr,const QIcon& appIcon = QIcon(),const QString& appName = "",const QString& appVersion = "",const QString& appInfo = "");
    ~KAboutDialog();

    /**
     * @brief 设置应用程序图标
     * @param icon
     */
    void setAppIcon(const QIcon& icon);

    /**
     * @brief 设置应用程序名称
     * @param appName
     */
    void setAppName(const QString& appName);

    /**
     * @brief 获取应用程序名称
     * @return
     */
    QString appName();

    /**
     * @brief 设置应用程序版本号
     * @param appVersion
     */
    void setAppVersion(const QString& appVersion);

    /**
     * @brief 获取应用程序版本号
     * @return
     */
    QString appVersion();

    /**
     * @brief 设置具体的说明内容
     * @param bodyText
     */
    void setBodyText(const QString& bodyText);

    /**
     * @brief 获取具体的说明内容
     * @return
     */
    QString bodyText();

    /**
     * @brief 设置服务与支持邮箱，有默认缺省
     * @param appSupport
     */
    void setAppSupport(const QString& appSupport);

    /**
     * @brief 获取服务与支持邮箱
     * @return
     */
    QString appSupport();

    /**
     * @brief 设置是否显示说明内容,应设计要求，1.2.0.9中不再显示说明内容
     * @param flag
     */
    void setBodyTextVisiable(bool flag);

    /**
     * @brief 设置隐私按钮是否可见
     * @param flag
     * @since 1.2
     */
    void setAppPrivacyLabelVisible(bool flag);

    /**
     * @brief 返回隐私按钮是否可见
     * @return
     * @since 1.2
     */
    bool AppPrivacyLabelIsVisible();

protected:
    void changeTheme() override;
    void paintEvent(QPaintEvent*paintEvent) override;

private:
    Q_DECLARE_PRIVATE(KAboutDialog)
    KAboutDialogPrivate* const d_ptr;
};
}
/**
  * @example testDialog/widget.h
  * @example testDialog/widget.cpp
  * @}
  */
#endif
