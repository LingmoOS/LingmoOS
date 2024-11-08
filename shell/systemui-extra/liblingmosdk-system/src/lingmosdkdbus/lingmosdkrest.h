/*
 * liblingmosdk-system's Library
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#ifndef LINGMOSDKREST_H
#define LINGMOSDKREST_H

/**
 * @file lingmosdkrest.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief sdk系统层屏幕分辨率信息生成dbus信号
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-system-dbus
 * @{
 * 
 */

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QtDebug>
#include <string.h>

#include <memory>

/**
 * @brief 生成屏幕分辨率信息dbus信号类
 * 主要用来生成系统当前显示屏幕,分辨率,系统支持分辨率dbus信号
 */
class KySdkRest : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.lingmosdk.resolution")
public:
    explicit KySdkRest(QObject *parent = nullptr);

public slots:
    /**
     * @brief  获取系统当前显示屏幕,分辨率,系统支持分辨率
     * 
     * @return QStringList 当前显示屏幕,分辨率,系统支持分辨率信息列表
     */
    QStringList getSysLegalResolution() const;

};

#endif // LINGMOSDKREST_H
