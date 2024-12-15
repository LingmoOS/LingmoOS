// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DEVICESTORAGE_H
#define DEVICESTORAGE_H
#include <QMap>
#include <QProcess>

namespace DiskManager {
class DeviceStorage
{
public:
    DeviceStorage();

    /**@brief:添加从hwinfo中获取的信息*/
    bool getDiskInfoFromHwinfo(const QString &devicePath);

    /**@brief:添加从lshw中获取的信息*/
    bool getDiskInfoFromLshw(const QString &devicePath);

    /**@brief:添加lsblk获取到的介质信息*/
    bool getDiskInfoFromLsblk(const QString &devicePath);

    /**@brief:添加smartctl中获取到的信息*/
    bool getDiskInfoFromSmartCtl(const QString &devicePath);

    /**@brief:获取当前磁盘model信息*/
    void getDiskInfoModel(const QString &devicePath, QString &model);

    /**@brief:获取当前磁盘介质信息*/
    QString getDiskInfoMediaType(const QString &devicePath);

    /**@brief:获取当前磁盘接口信息*/
    void getDiskInfoInterface(const QString &devicePath, QString &interface, QString &model);

    /**@brief:为HW设备更新信息*/
    void updateForHWDevice(const QString &devicePath);


private:
    void getMapInfoFromInput(const QString &info, QMap<QString, QString> &mapInfo);

    void getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    void getMapInfoFromLshw(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    void loadLsblkInfo(const QString &info, QMap<QString, QString> &mapInfo);

    void getMapInfoFromSmartctl(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch = QString(": "));

    /**@brief:解析从 sudo hwinfo --disk 中获取的消息*/
    bool setHwinfoInfo(const QMap<QString, QString> &mapInfo);
    /**@brief:解析从 sudo hwinfo --disk 中获取的消息,KLU专用*/
    bool setKLUHwinfoInfo(const QMap<QString, QString> &mapInfo);
    /**@brief:设置存储设备的介质类型*/
    bool setMediaType(const QString &name, const QString &value);
    /**@brief:设置存储设备的介质类型,KLU专用*/
    bool setKLUMediaType(const QString &name, const QString &value);

    /**@brief:解析从 sudo lshw -C disk 中获取的消息*/
    bool addInfoFromlshw(const QMap<QString, QString> &mapInfo);

    /**@brief:解析从 sudo smartctl --all /dev/sda中获取的消息*/
    bool addInfoFromSmartctl(const QMap<QString, QString> &mapInfo);

    /**@brief:从sudo lshw -C disk 中获取信息*/
    void getInfoFromLshw(const QMap<QString, QString> &mapInfo);

    /**@brief:从sudo smartctl --all /dev/sda 中获取信息*/
    void getInfoFromsmartctl(const QMap<QString, QString> &mapInfo);

    /**@brief:将属性设置到成员变量*/
    void setAttribute(const QMap<QString, QString> &mapInfo, const QString &key, QString &variable, bool overwrite = true);
public:
    QString               m_model;              //<! 【型号】1
    QString               m_vendor;             //<! 【制造商】2 //有可能会没有
    QString               m_mediaType;          //<! 【介质类型】3
    QString               m_size;               //<! 【大小】4
    QString               m_rotationRate;       //<! 【转速】
    QString               m_interface;          //<! 【接口】6
    QString               m_serialNumber;       //<! 【序列号】7
    QString               m_version;            //<! 【版本】
    QString               m_capabilities;       //<! 【功能】
    QString               m_description;        //<! 【描述】
    QString               m_powerOnHours;       //<! 【通电时间】9
    QString               m_powerCycleCount;    //<! 【通电次数】10
    QString               m_firmwareVersion;    //<! 【固件版本】8
    QString               m_speed;              //<! 【速度】5

    QString               m_DeviceFile;         //<   设备文件
    QString               m_KeyToLshw;          //<!  用于表示存储文件
    QString               m_KeyFromStorage;     //<!  有些其他设备与usb重复，需要用这个进行唯一标识
};

} // namespace DiskManager
#endif // DEVICESTORAGE_H
