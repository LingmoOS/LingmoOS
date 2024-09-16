// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICESTORAGE_H
#define DEVICESTORAGE_H
#include"DeviceInfo.h"

/**
 * @brief The DeviceStorage class
 * 用来描述存储设备的类
 */

class DeviceStorage: public DeviceBaseInfo
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceStorage)
public:
    DeviceStorage();

    /**
       * @brief unitConvertByDecimal:将m_Size的byte单位转换为合理单位：GB、TB之类
       */
    void unitConvertByDecimal();

  /**
      * @brief setInfoFromTomlOneByOne:设置从toml里面获取的信息
      * @param mapInfo:由toml获取的信息map
      * @return枚举值
      */
   TomlFixMethod setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo);
   
    /**
       * @brief setHwinfoInfo:设置由hwinfo --disk命令获取的设备信息
       * @param mapInfo:由hwinfo获取的信息map
       * @return 布尔值，true:信息设置成功；false:信息设置失败
       */
    bool setHwinfoInfo(const QMap<QString, QString> &mapInfo);

    /**
       * @brief setKLUHwinfoInfo:设置由hwinfo --disk命令获取的设备信息，KLU专用
       * @param mapInfo:由hwinfo获取的信息map
       * @return 尔值，true:信息设置成功；false:信息设置失败
       */
    bool setKLUHwinfoInfo(const QMap<QString, QString> &mapInfo);

    /**
     * @brief setMediaType:设置存储设备的介质类型
     * @param name:存储设备逻辑名称
     * @param value:类型值0/1
     * @return 布尔值:true-设置成功；false--设置失败
     */
    bool setMediaType(const QString &name, const QString &value);

    /**
     * @brief setKLUMediaType:设置存储设备的介质类型,KLU专用
     * @param name:存储设备逻辑名称
     * @param value:类型值0/1
     * @return 布尔值:true-设置成功；false--设置失败
     */
    bool setKLUMediaType(const QString &name, const QString &value);

    /**
     * @brief addInfoFromlshw:将lshw获取的信息与存储设备进行匹配
     * @param mapInfo:由lshw获取的信息map
     * @return 布尔值:true-设置成功；false--设置失败
     */
    bool addInfoFromlshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief addNVMEInfoFromlshw:将lshw获取的信息与存储设备进行匹配
     * @param mapInfo:由lshw获取的信息map
     * @return 布尔值:true-设置成功；false--设置失败
     */
    bool addNVMEInfoFromlshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief addInfoFromSmartctl:将sudo smartctl --all /dev/xxx中获取的信息与存储设备进行匹配
     * @param name:存储设备逻辑名称
     * @param mapInfo:由smartctl获取的信息map
     * @return 布尔值:true-设置成功；false--设置失败
     */
    bool addInfoFromSmartctl(const QString &name, const QMap<QString, QString> &mapInfo);

    /**
     * @brief getInfoFromLshw:设置由lshw获取的信息
     * @param mapInfo:lshw获取的信息map
     */
    void getInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**
     * @brief getInfoFromsmartctl:设置由smartctl获取的信息
     * @param mapInfo:smartctl获取的信息map
     */
    void getInfoFromsmartctl(const QMap<QString, QString> &mapInfo);

    /**
     * @brief isValid:判断设备信息是否有效
     * @return 布尔值:true-设备信息有效；false--设备信息无效
     */
    bool isValid();

    /**
     * @brief setDiskSerialID:设置序列号属性 from Device Files
     * @param deviceFiles:hwinfo --disk中获取的device Files属性值
     */
    void setDiskSerialID(const QString &deviceFiles);

    /**
     * @brief getDiskSerialID:获取序列号属性
     * @return 序列号属性
     */
    QString getDiskSerialID();

    /**
     * @brief device:追加磁盘
     * @param device:磁盘信息
     */
    void appendDisk(DeviceStorage *device);

    /**
     * @brief getDiskSizeByte:读取磁盘大小，单位Byte
     */
    virtual quint64 getDiskSizeByte() { return m_SizeBytes; }

    /**
     * @brief checkDiskSize:规范磁盘大小
     */
    virtual void checkDiskSize();

    /**
     * @brief compareSize:比较nvme提供的两个容量值的大小
     * @param size1:Total NVM Capacity
     * @param size2:Namespace 1 Size/Capacity
     * @return 返回较大的容量值
     */
    QString compareSize(const QString &size1, const QString &size2);

    const QString &name()const override;

    /**
     * @brief name:获取制造商属性值
     * @return QString 制造商属性值
     */
    const QString &vendor()const override;

    const QString &driver()const override;

    /**
       * @brief keyFromStorage:获取存储设备标识符
       * @param 无
       * @return QString:存储设备标识符
       */
    const QString &keyFromStorage()const;

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
     * @brief loadTableHeader : 过去表格的表头数据
     */
    void loadTableHeader() override;

    /**
     * @brief loadTableData:加载表头信息
     */
    void loadTableData() override;

private:
    /**
     * @brief getSerialID 获取序列号字符串
     * @param strDeviceLink  DeviceLink
     * @return Serial字符串
     */
    QString getSerialID(QString &strDeviceLink);


private:
    QString               m_Model;              //<! 【型号】1
    // QString               m_Vendor;             //<! 【制造商】2
    QString               m_MediaType;          //<! 【介质类型】3
    QString               m_Size;               //<! 【大小】4
    quint64               m_SizeBytes;               //<! 【大小单位byte】4  //对于有合并的时候 用此代替m_Size进行合并更准确。
    QString               m_RotationRate;       //<! 【转速】
    QString               m_Interface;          //<! 【接口】6
    QString               m_SerialNumber;       //<! 【序列号】7
    QString               m_Capabilities;       //<! 【功能】
    QString               m_FirmwareVersion;    //<! 【固件版本】8
    QString               m_Speed;              //<! 【速度】5


    QString               m_DeviceFile;         //<
    QString               m_KeyToLshw;          //<!  用于表示存储文件
    QString               m_KeyFromStorage;     //<!  有些其他设备与usb重复，需要用这个进行唯一标识

    QString               m_NvmeKey;             //<!  用于表示NVME的存储文件
};

#endif // DEVICESTORAGE_H
