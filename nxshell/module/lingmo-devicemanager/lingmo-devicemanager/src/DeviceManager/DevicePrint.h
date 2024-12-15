// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEPRINT_H
#define DEVICEPRINT_H
#include"DeviceInfo.h"

/**
 * @brief The DevicePrint class
 * 用来描述打印机的类
 */

class DevicePrint : public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DevicePrint)
public:
    DevicePrint();

  /**
      * @brief setInfoFromTomlOneByOne:设置从toml里面获取的信息
      * @param mapInfo:由toml获取的信息map
      * @return枚举值
      */
   TomlFixMethod setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo);
   
    /**
     * @brief setInfo:设置打印机信息
     * @param info:由cups第三方库获取的打印机信息map
     */
    void setInfo(const QMap<QString, QString> &info);

    /**
     * @brief name:获取名称属性值
     * @return QString 名称属性值
     */
    const QString &name()const override;

    /**
     * @brief name:获取制造商属性值
     * @return QString 制造商属性值
     */
    const QString &vendor()const override;

    /**
     * @brief name:获取型号属性值
     * @return QString 型号属性值
     */
    const QString makeAndeModel()const;

    /**
     * @brief driver 返回驱动，重写纯虚函数
     * @return
     */
    const QString &driver() const override;

    /**
     * @brief available 返回是否可用
     * @return
     */
    bool available() override;

    /**
     * @brief subTitle:获取子标题
     * @return 子标题
     */
    QString subTitle() override;

    /**
     * @brief getOverviewInfo:获取概况信息
     * @return 概况信息
     */
    const QString getOverviewInfo() override;

    /**
     * @brief setEnable 设置设备的禁用状态
     * @param enable 是否禁用
     * @return 禁用是否成功
     */
    EnableDeviceStatus setEnable(bool enable) override;

    /**
     * @brief enable : 判断当前是否是禁用状态
     * @return
     */
    bool enable() override;


    /**
     * @brief getVendor 获取制造商
     * @return
     */
    inline QString getVendor() { return m_Vendor; }

    /**
     * @brief getModel 获取型号
     * @return
     */
    inline QString getModel() { return m_Model; }
protected:

    /**
     * @brief initFilterKey:初始化可现实的可显示的属性,m_FilterKey
     */
    void initFilterKey() override;

    /**
     * @brief loadBaseDeviceInfo:加载基本信息
     */
    void loadBaseDeviceInfo() override;

    /**
     * @brief loadOtherDeviceInfo:加载基本信息
     */
    void loadOtherDeviceInfo() override;

    /**
     * @brief loadTableData:加载表头信息
     */
    void loadTableData() override;

private:
    QString             m_Model;                        //<! 【型号】
    QString             m_SerialNumber;                 //<! 【序列号】
    QString             m_InterfaceType;                //<! 【接口类型】
    QString             m_URI;                          //<! 【URI】
    QString             m_Status;                       //<! 【状态】
    QString             m_Shared;                       //<! 【共享】
    QString             m_MakeAndModel;                 //<! 【制造商和型号】
};

#endif // DEVICEPRINT_H
