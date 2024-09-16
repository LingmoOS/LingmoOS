// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file httpadvancedsettingwidget.h
 *
 * @brief http高级设置
 *
 *@date 2020-07-29 11:00
 *
 * Author: bulongwei  <bulongwei@uniontech.com>
 *
 * Maintainer: bulongwei  <bulongwei@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HTTPADVANCEDSETTINGWIDGET_H
#define HTTPADVANCEDSETTINGWIDGET_H
#include <ddialog.h>
#include <QDebug>
#include <DButtonBox>
#include <DTextEdit>
#include <QSettings>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

const QString UOS_DONWLOAD_HTTPADVANCED_PATH = "/usr/share/downloader/database/"; //备份数据库路径

class HttpAdvancedSettingWidget : public DDialog
{
    Q_OBJECT

public:
    explicit HttpAdvancedSettingWidget(QWidget *parent = nullptr);

    /**
     * @brief 恢复默认
     */
    void reset();

private:
    /**
     * @brief 初始化ui
     */
    void initUI();

private slots:
    /**
     * @brief 选择后缀名管理按钮
     */
    void onSuffixBtnClicked();
    /**
     * @brief 选择网站管理按钮
     */
    void onWebBtnClicked();
    /**
     * @brief 恢复默认后缀名管理按钮
     */
    void onRstoreDefaultClicked();
    /**
     * @brief 确定按钮
     */
    void onSureClicked();
    /**
     * @brief 取消按钮
     */
    void onCancelClicked();


private:
    DTextEdit *m_textEdit;
    DButtonBox *m_btnBox;
    DCommandLinkButton * m_defaultBtn;
    QString m_configPath;

    QString m_curSuffixStr;
    QString m_curWebStr;
};



#endif // HTTPADVANCEDSETTINGWIDGET_H
