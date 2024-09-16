// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file filesavepathchooser.h
 *
 * @brief 下载目录窗口，主要实现切换单选按钮时，文件保存路径控件的使能以及设置数据的传递与显示
 *
 *@date 2020-06-09 11:01
 *
 * Author: yuandandan  <yuandandan@uniontech.com>
 *
 * Maintainer: yuandandan  <yuandandan@uniontech.com>
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
#ifndef FILESAVEPATHCHOOSER_H
#define FILESAVEPATHCHOOSER_H

#include <DRadioButton>
#include <dfilechooseredit.h>

#include <QWidget>

DWIDGET_USE_NAMESPACE

/**
 * @class FileSavePathChooser
 * @brief 下载目录窗口类
*/
class FileSavePathChooser : public QWidget
{
    Q_OBJECT
public:
    explicit FileSavePathChooser(const int &currentSelect, const QString &downloadPath);

    /**
     * @brief 设置文件保存路径
     * @param text 文件路径
     */
    bool setLineEditText(const QString &text);

    /**
     * @brief 设置当前选中按钮，当为1时，选中上次使用目录按钮
     * @param currentSelect 当前选中值
     */
    bool setCurrentSelectRadioButton(const int &currentSelect);

signals:
    void textChanged(QString text);

public slots:

    /**
     * @brief 单选按钮选择响应的槽函数
     */
    void onRadioButtonClicked();

    /**
     * @brief 文件保存路径改变响应的槽函数
     * @param text 改变后的路径
     */
    void onLineEditTextChanged(const QString &text);

private:
    /**
     * @brief 初始化界面
     */
    void initUI();

    /**
     * @brief 初始化信号连接
     */
    void initConnections();

private:
    DFileChooserEdit *m_fileChooserEdit; // 文件保存路径选择
    DRadioButton *m_autoLastPathRadioButton; // 自动修改为上次使用的目录按钮
    DRadioButton *m_customsPathRadioButton; // 设置默认目录按钮
    int m_currentSelect; // 当前选中按钮值
    QString m_downloadPath; // 当前保存路径
};

#endif // FILESAVEPATHCHOOSER_H
