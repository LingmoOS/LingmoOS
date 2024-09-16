// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/namespace.h"
#include "dtkwidget_global.h"
#include "../../deepin-defender/src/window/modules/common/common.h"

#include <QWidget>
#include <QThread>
#include <QSettings>

class PolicyKitHelper;
class InvokerFactoryInterface;
class DBusInvokerInterface;
class SettingsInvokerInterface;

DEF_NAMESPACE_BEGIN
DWIDGET_USE_NAMESPACE

class UsbConnectionWork : public QObject
{
    Q_OBJECT
public:
    explicit UsbConnectionWork(InvokerFactoryInterface *factory, QObject *parent = nullptr);
    ~UsbConnectionWork();
    inline bool isbSaveRecord() const { return m_isbSaveRecord; }
    inline QString limitationModel() const { return m_limitationModel; }
    inline QList<UsbConnectionInfo> &usbConnectionInfoLst() { return m_usbConnectionInfoLst; }
    inline QList<UsbConnectionInfo> &usbConnectionWhiteLst() { return m_usbConnectionWhiteLst; }

    void clearUsbConnectionLog();
    void clearSrvUsbConnectionLog();
    void appendToUsbConnectionWhiteLst(UsbConnectionInfo info);
    void removeFromUsbConnectionWhiteLst(UsbConnectionInfo info);
    bool isUsbDeviceInWhiteList(const QString &serial, const QString &vendor, const QString &product);

    // 设置所有设置白名单操作的按钮状态
    bool getAllOperatingBtnStatus() const { return m_enableAllOperatingBtn; }
    void setAllOperatingBtnStatus(bool enable) { m_enableAllOperatingBtn = enable; }

private:
    void loadConfig();
    //读取usb连接记录
    void readConnectionRecord();
    //读取usb连接白名单
    void readConnectionWhiteLst();
    void updateNameInWhiteLst(QString sName, QString sSerial, QString sVendorID, QString sProductID);
    //cleanUsbConnectionLog:超过30天
    void cleanUsbConnectionLog();

Q_SIGNALS:
    //发送需要添加到日志表格的信息
    void sendAppendUsbConnectionLogInfo(const UsbConnectionInfo &info);
    void notifyOpenUsbConnectionWindow();

    // 请求确认是否改变连接记录开关
    void requestConfirmChangeIsbSaveRecord();
    // 通知去改变连接记录开关
    void notifyChangeIsbSaveRecord();
    // 请求确认选择usb限制模式
    void requestConfirmSelectLimitationModel(const QString &limitModel);
    // 通知去改变usb限制模式
    void notifyChangeLimitationModel(const QString &limitModel);
    // 请求确认添加到白名单
    void requestConfirmAddUsbConnectionWhiteLst(const UsbConnectionInfo &info);
    // 通知去添加到白名单
    void notifyAddUsbConnectionWhiteLst(const UsbConnectionInfo &info);
    // 请求确认清除usb连接记录
    void requestConfirmClearUsbConnectionLog();
    // 通知去清除usb连接记录
    void notifyClearUsbConnectionLog();
    // 权限校验完成
    void checkAuthorizationFinished();

public Q_SLOTS:
    void changeIsbSaveRecord();
    void selectLimitationModel(const QString &modelStr);
    // 当接收到设备接入的信息
    void onReceiveUsbInfo(const QString &sInfos);

private:
    //初始配置
    bool m_isbSaveRecord;
    QString m_limitationModel;
    bool m_enableAllOperatingBtn; // 所有操作按钮是否可用

    QList<UsbConnectionInfo> m_usbConnectionInfoLst;
    QList<UsbConnectionInfo> m_usbConnectionWhiteLst;

    DBusInvokerInterface *m_monitorNetFlowInvokerInter;
    QThread *m_polKitHelperThread;
    PolicyKitHelper *m_policyKitHelper;
};

DEF_NAMESPACE_END
