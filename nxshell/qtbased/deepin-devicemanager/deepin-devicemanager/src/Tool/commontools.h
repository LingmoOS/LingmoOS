// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONTOOLS_H
#define COMMONTOOLS_H

#include "MacroDefinition.h"

#include <QObject>
#include <QMap>
#include <QPixmap>
#include <QIcon>

#include <DWidget>

class CommonTools : public QObject
{
    Q_OBJECT
public:
    explicit CommonTools(QObject *parent = nullptr);

    /**
     * @brief getDriverType: 获取驱动类型对应的字符串
     * @param type 驱动类型
     * @return QString
     */
    static QString getDriverType(DriverType type);

    /**
     * @brief getDriverPixmap: 获取驱动类型对应的图标
     * @param type 驱动类型
     * @return
     */
    static QString getDriverPixmap(DriverType type);

    /**
     * @brief getStausType: 获取状态类型对应的字符串
     * @param type 状态类型
     * @return QString
     */
    static QString getStausType(Status type);

    /**
     * @brief getStatusPixmap: 获取状态类型类型对应的图标
     * @param type 状态类型
     * @return
     */
    static QString getStatusPixmap(Status type);

    /**
     * @brief getSystemTime: 获取系统时间
     * @return
     */
    static QString getSystemTime();

    /**
     * @brief getErrorString
     * @param err
     * @return
     */
    static QString getErrorString(int err);

    /**
     * @brief feedback
     */
    static void feedback();

    /**
     * @brief getUrl
     * @return
     */
    static QString getUrl();

    /**
     * @brief getBackupPath 备份路径
     */
    static QString getBackupPath();

signals:

public slots:

public:
    static QMap<DriverType, QString> m_MapDriverIcon;
    static QMap<DriverType, QString> m_MapDriverType;
    static QMap<Status, QString>     m_MapStatusIcon;
    static QMap<Status, QString>     m_MapStatusType;

    static QMap<int, QString>     m_MapErrorString;
};

#endif // COMMONTOOLS_H
