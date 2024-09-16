// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file clipboardtimer.h
 *
 * @brief 剪切板托管类
 *
 * @date 2020-06-09 10:51
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

#ifndef CLIPBOARDTIMER_H
#define CLIPBOARDTIMER_H
#include "settings.h"
#include <DObject>
/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file clipboardtimer.h
 *
 * @brief 剪切板
 *
 * @date 2020-08-31 14:11
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
#include <QObject>
#include <QApplication>
#include <QSystemTrayIcon>

class QClipboard;
class QMimeData;

class ClipboardTimer : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardTimer(QObject *parent = nullptr);
    ~ClipboardTimer();

    /**
     * @brief 检测剪切板是否有url
     */
    void checkClipboardHasUrl();

private:
    /**
     * @brief isMagnet 是否是磁力链接
     * @param str url链接
     */
    bool isMagnetFormat(QString str);
    /**
     * @brief isMagnet 是否是http+后缀
     * @param str url链接
     */
    bool isHttpFormat(QString str);
    /**
     * @brief isBt 是否是bt后缀
     * @param str url链接
     */
    bool isBtFormat(QString str);

    /**
     * @brief isMl 是否是Ml后缀
     * @param str url链接
     */
    bool isMlFormat(QString str);

    /**
     * @brief getTypeList 获取配置文件中的type
     * @return 返回解析后的后缀名列表
     */
    QStringList getTypeList();

    /**
     * @brief getWebList 获取配置文件中的不接管网站
     * @return 返回解析后的不接管网站列表
     */
    QStringList getWebList();

    /**
     * @brief isWebFormat 是否符合不接管网站
     * @param str url链接
     * @return bool 真假
     */
    bool isWebFormat(QString url);

    /**
     * @brief isWebFormat 是否符合不接管网站
     * @param str url链接
     * @return bool 真假
     */
    QStringList midWebList(QStringList url);

private slots:
    /**
     * @brief getDataChanged 获取剪切板内容
     */
    void getDataChanged();

signals:
    /**
     * @brief getDataChanged 发送剪切板内容至主界面
     */
    emit void sendClipboardTextChange(QString url);
private:
    QClipboard *m_clipboard; //剪切板
    QByteArray m_timeStamp;  //记录上次剪切板时间
    QString m_sessionType;  //当前窗管版本  x11 wayland
    QString m_lastUrl;  //记录剪切板上次内容
};

#endif // CLIPBOARDTIMER_H
