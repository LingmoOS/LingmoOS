// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USBMANAGEMONITOR_H
#define USBMANAGEMONITOR_H

#include "../../deepin-defender/src/window/modules/common/common.h"

#include <QObject>
#include <QMutex>

class SettingsSql;
class UsbConnLogSql;
class UsbWhitelistSql;

class UsbManageMonitor : public QObject
{
    Q_OBJECT
public:
    explicit UsbManageMonitor(QObject *parent = nullptr);
    ~UsbManageMonitor();
    void init();
    void run();
    void quit();
    // 将待发送设备信息整合成字符串
    static QString mergeDevSavingInfoToStr(const QString &sName, const QString &sStatus, const QString &sSerial, const QString &sVendorID,
                                           const QString &sProductID, const QString &sParentSize, const QString &sSize);

    void writeConnectionRecord(const QString &name, const QString &sDatetime, const QString &status,
                               const QString &serial, const QString &vendorID, const QString &productID,
                               const QString &sParentSize, const QString &sSize);
    void clearUsbConnectionLog();
    void cleanUsbConnectionLog();

    void writeConnectionWhiteLst(const QString &name, const QString &sDatetime, const QString &status,
                                 const QString &serial, const QString &vendorID, const QString &productID,
                                 const QString &sParentSize, const QString &sSize);
    void removeFromWhiteLst(const QString &sSerial, const QString &sVendorID, const QString &sProductID);

    void changeIsbSaveRecord(bool change);
    void selectLimitationModel(const QString &modelStr);
    inline bool getRecordSaveSwitch() const { return m_isbSaveRecord; }
    inline QString getLimitModel() const { return m_sLimitModel; }
    QString getUsbConnectionRecords();
    QString getWhiteList();

private:
    //读取配置数据
    void loadConfig();
    void readConnectionRecord();
    void readConnectionWhiteLst();
    //首次启动弹出当前可移动设备
    void disconnectAllDevices();
    // 首次启动按白名单对usb设备进行断开处理
    void preManage();
    // 判断是否处于授权管理激活审核模式
    bool isActivatorInVerifingModel();
    void monitorDevices(struct udev *udev);
    void processDevice(struct udev_device *dev);
    // 检查设备是否在白名单中，存在则返回0
    int checkWhitelist(const QString &sSerial, const QString &sIdVendor, const QString &sIdProduct);
    // 获取分区标签
    QString getDevLabel(const QString &sDevNode);
    // 获取设备容量大小
    QString getDevSize(const QString &sDevNode);
    // 获取设备文件系统类型
    QString getDevFsType(const QString &sDevNode);
    // 更新白名单中设备名称
    void updateNameInWhiteLst(const QString &sName, const QString &sSerial, const QString &sVendorID, const QString &sProductID);
    // 由节点路径获取设备信息
    bool getDevInfoFromDevNode(UsbDeviceInfo &info, const QString &devNode);
    // 断开设备
    bool disconnectDevBySysPath(const QString &sysPath);
    // 根据策略管控设备
    void manageDevByPolicy(const UsbDeviceInfo &info);
Q_SIGNALS:
    //usb设备接入的信号，入参bflag 为false,表示限制状态
    void checkUsbInsert(bool bFlag, QString sUsbInfo);
    void sendConnectionRecord(QString sInfos);
    //通知显示限制所有usb设备的系统通知
    void requestShowBlockAllSysNotify();
    //通知显示限制非白名单usb设备的系统通知
    void requestShowOnlyWhiteListSysNotify();
public slots:

private:
    QMutex m_mutex;
    struct udev *m_udev;
    //配置文件路径
    QString m_configPath;
    //运行状态
    int m_status; //1-运行 0-停止
    //是否保存记录标志
    bool m_isbSaveRecord;
    //limitation model
    QString m_sLimitModel;

    SettingsSql *m_settingsSql;
    UsbConnLogSql *m_usbConnLogSql;
    UsbWhitelistSql *m_usbWhitelistSql;

    QList<UsbConnectionInfo> m_usbConnectionInfoLst;
    QList<UsbConnectionInfo> m_whiteList;
};

#endif // USBMANAGEMONITOR_H
