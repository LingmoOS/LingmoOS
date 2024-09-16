// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file func.h
 *
 * @brief 方法类
 *
 * @date 2020-08-28 09:50
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

#ifndef FUNC_H
#define FUNC_H

#include <QObject>

class Func : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 网络是否联通
     * @return true为不通 false为通
     */
    static bool isNetConnect();

    /**
     * @brief http是否联通
     * @return true为不通 false为通
     */
    static bool isHTTPConnect();

    /**
     * @brief http是否联通
     * @return true为不通 false为通
     */
    static bool isIPV6Connect();

    /**
     * @brief 设置Mimeapps文件中的值，如果key不存在，新建key。
     * @param key 配置中的key，如果key不存在新建key
     * @param value key对应值。
     * @return 是否设置成功
     */
    static bool setMimeappsValue(QString key, QString value = "");

    /**
     * @brief 获取文件的md5值
     * @param path 文件路径。
     * @return md5值
     */
    static QString pathToMD5(QString path);

    /**
     * @brief 获取*.ini文件的指
     * @param path 文件路径。
     * @param group 组
     * @param key 键
     * @return 值
     */
    static QString getIniConfigValue(QString path, QString group, QString key);

    /**
     * @brief 设置*.ini文件的指
     * @param path 文件路径。
     * @param group 组
     * @param key 键
     * @param value 值
     * @return 是否写入成功
     */
    static bool setIniConfigValue(QString path, QString group, QString key, QString value);

    /**
     * @brief 速度字符串转数值
     * @param str 速度
     */
    static double formatSpeed(QString str);

    /**
     * @brief 文件大小字符串转数值
     * @param str 文件大小
     */
    static double formatFileSize(QString str);

    /**
     * @brief 将字符串中的中文转成拼音
     * @param input 输入的字符串
     */
    static QString chineseToPinyin(QString input);

//    /**
//     * @brief 设置最大打开文件个数（全局最大同事下载资源数）
//     * @param maxLen 个数
//     */
//    static bool setfdLimit(unsigned long maxLen);
private:

    /**
     * @brief 删除拼音中的数字
     * @param input 带数字的拼音
     */
    static QString removeDigital(QString input);

    /**
     * @brief 判断当前电脑是否连接局域网
     * @retrun 是否连接, true为连接, false为未连接
     */
    static bool isLanConnect();
};

#endif // FUNC_H
