// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEINFO_H
#define PAGEINFO_H

#include <QObject>
#include <DWidget>

class DeviceBaseInfo;

using namespace Dtk::Widget;

/**
 * @brief The PageInfo class
 * UI 设备信息展示的基类
 */
class PageInfo : public DWidget
{
    Q_OBJECT
public:
    explicit PageInfo(QWidget *parent = nullptr);

    /**
     * @brief updateTable : 更新表格
     * @param lst : 某类型设备列表
     */
    virtual void updateInfo(const QList<DeviceBaseInfo *> &lst) = 0;

    /**
     * @brief updateInfo:刷新概况信息
     * @param map : 概况信息
     */
    virtual void updateInfo(const QMap<QString, QString> &map);


    /**
     * @brief clearWidgets clear widgets
     */
    virtual void clearWidgets() = 0;

    /**
     * @brief setLabel:设置Label
     * @param itemstr:Label信息
     */
    virtual void setLabel(const QString &itemstr) = 0;

    /**
     * @brief setLabel:设置图片Label，设备Label，操作系统Label
     * @param str1:设备名称
     * @param str2:操作系统信息
     */
    virtual void setLabel(const QString &str1, const QString &str2);

    /**
     * @brief clearContent:清除内容
     */
    virtual void clearContent();

    /**
     * @brief isOverview:是否是概况界面
     * @return false : 不是概况界面
     */
    virtual bool isOverview() {return false;}

    /**
     * @brief isBaseBoard : 是否是主板信息界面
     * @return false : 不是主板信息界面
     */
    virtual bool isBaseBoard() {return false;}

    /**
     * @brief setDeviceInfoNum 设置设备信息数目
     * @param num
     */
    virtual void setDeviceInfoNum(int num);

    /**
     * @brief getDeviceInfoNum 获取设备信息数目
     * @return
     */
    virtual int getDeviceInfoNum();

    /**
     * @brief setDeviceInfoNum 设置多行标识
     * @return
     */
    virtual void setMultiFlag(const bool &flag);

    /**
     * @brief getMultiFlag 获取多行标识
     * @return
     */
    virtual bool getMultiFlag();

    /**
     * @brief packageHasInstalled 是否被安装
     * @param packageName 包名
     * @return
     */
    static bool packageHasInstalled(const QString &packageName);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    int m_AllInfoNum;
    bool m_multiFlag;  // 多行标识

};

#endif // PAGEINFO_H
