// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020} Uniontech Technology Co., Ltd.
 *
 * @file aria2cbtinfo.cpp
 *
 * @brief bt 信息类
 *
 * @date 2020-05-26 11:55
 *
 * Author: denglinglong  <denglinglong@uniontech.com>
 *
 * Maintainer: denglinglong  <denglinglong@uniontech.com>
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

#ifndef ARIA2CBTINFO_H
#define ARIA2CBTINFO_H
#include <QObject>

class Aria2cBtFileInfo
{
public:
    int index; //文件索引,从1开始
    QString path; //文件路径,包含文件夹层次
    QString length; //文件大小
    long lengthBytes; //文件大小(字节)
};

class Aria2cBtInfo
{
public:
    Aria2cBtInfo();

public:
    QString mode; //模式,multi表示有多个文件,signal表示单个文件
    QStringList announceList; //种子服务器列表
    QString infoHash; //hash特征值
    QString pieceLength; //分片大小
    int pieceNumber; //分片数量
    QString totalLength; //总大小(带单位)
    long totalLengthByets = 0; //总大小(字节)
    QString name; //种子名称(多文件时为文件夹名,单文件为文件名)
    QString magnet; //对应的磁力链接
    QList<Aria2cBtFileInfo> files; //文件列表
};

#endif // ARIA2CBTINFO_H
