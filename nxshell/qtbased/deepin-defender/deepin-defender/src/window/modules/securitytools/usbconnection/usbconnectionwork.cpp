// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbconnectionwork.h"
#include "policykithelper.h"
#include "../src/window/modules/common/invokers/invokerfactory.h"
#include "../src/window/modules/common/invokers/invokerinterface.h"

#include <DDialog>

#include <QDir>
#include <QDebug>

#define USB_INFO_SIZE 8 // usb信息种类个数

UsbConnectionWork::UsbConnectionWork(InvokerFactoryInterface *factory, QObject *parent)
    : QObject(parent)
    , m_isbSaveRecord(true)
    , m_limitationModel(USB_LIMIT_MODEL_ALL)
    , m_enableAllOperatingBtn(true)
    , m_monitorNetFlowInvokerInter(nullptr)
    , m_polKitHelperThread(nullptr)
    , m_policyKitHelper(nullptr)
{
    qRegisterMetaType<UsbConnectionInfo>("UsbConnectionInfo");

    // invoker
    m_monitorNetFlowInvokerInter = factory->CreateInvoker("com.deepin.defender.MonitorNetFlow",
                                                          "/com/deepin/defender/MonitorNetFlow",
                                                          "com.deepin.defender.MonitorNetFlow",
                                                          ConnectType::SYSTEM, this);
    m_monitorNetFlowInvokerInter->Connect("SendUsbConnectionInfo", this, SLOT(onReceiveUsbInfo(QString)));

    loadConfig();

    //读取白名单
    readConnectionWhiteLst();
    //读取usb连接记录
    readConnectionRecord();
    //清除超过30天记录
    cleanUsbConnectionLog();

    // thread
    m_polKitHelperThread = new QThread;
    m_policyKitHelper = new PolicyKitHelper;
    m_policyKitHelper->moveToThread(m_polKitHelperThread);
    /// 连接从m_policyKitHelper发出的信号
    // 通知去改变连接记录开关
    connect(m_policyKitHelper, &PolicyKitHelper::notifyChangeIsbSaveRecord,
            this, &UsbConnectionWork::changeIsbSaveRecord);
    // 通知去改变usb限制模式
    connect(m_policyKitHelper, &PolicyKitHelper::notifyChangeLimitationModel,
            this, &UsbConnectionWork::selectLimitationModel);
    // 通知去添加到白名单
    connect(m_policyKitHelper, &PolicyKitHelper::notifyAddUsbConnectionWhiteLst,
            this, &UsbConnectionWork::notifyAddUsbConnectionWhiteLst);
    // 通知去清除usb连接记录
    connect(m_policyKitHelper, &PolicyKitHelper::notifyClearUsbConnectionLog,
            this, &UsbConnectionWork::notifyClearUsbConnectionLog);
    // 权限校验完成
    connect(m_policyKitHelper, &PolicyKitHelper::checkAuthorizationFinished, this, &UsbConnectionWork::checkAuthorizationFinished);
    m_polKitHelperThread->start();

    /// 连接从UsbConnectionWork发出的信号
    // 请求确认是否改变连接记录开关
    connect(this, &UsbConnectionWork::requestConfirmChangeIsbSaveRecord,
            m_policyKitHelper, &PolicyKitHelper::confirmChangeIsbSaveRecord);
    // 请求确认选择usb限制模式
    connect(this, &UsbConnectionWork::requestConfirmSelectLimitationModel,
            m_policyKitHelper, &PolicyKitHelper::confirmSelectLimitationModel);
    // 请求确认添加到白名单
    connect(this, &UsbConnectionWork::requestConfirmAddUsbConnectionWhiteLst,
            m_policyKitHelper, &PolicyKitHelper::confirmAddUsbConnectionWhiteLst);
    // 请求确认清除usb连接记录
    connect(this, &UsbConnectionWork::requestConfirmClearUsbConnectionLog,
            m_policyKitHelper, &PolicyKitHelper::confirmClearUsbConnectionLog);
}

UsbConnectionWork::~UsbConnectionWork()
{
    // 完成后删除线程
    m_polKitHelperThread->quit();
    m_polKitHelperThread->wait();
    m_polKitHelperThread->deleteLater();
    m_policyKitHelper->deleteLater();
}

void UsbConnectionWork::loadConfig()
{
    m_isbSaveRecord = m_monitorNetFlowInvokerInter->Invoke("GetRecordSaveSwitch", {}, BlockMode::BLOCK)
                          .arguments()
                          .first()
                          .toBool();
    m_limitationModel = m_monitorNetFlowInvokerInter->Invoke("GetLimitModel", {}, BlockMode::BLOCK)
                            .arguments()
                            .first()
                            .toString();
}

void UsbConnectionWork::readConnectionRecord()
{
    //name--time--isbBlocked--serial--vendorID--productID--sParentSize--sSize
    QString sRecords = m_monitorNetFlowInvokerInter->Invoke("GetUsbConnectionRecords", {}, BlockMode::BLOCK)
                           .arguments()
                           .first()
                           .toString();

    QTextStream txtOutput(&sRecords);
    QString lineStr;
    while (!txtOutput.atEnd()) {
        lineStr = txtOutput.readLine();
        QStringList infoLst = lineStr.split(",");
        if (USB_INFO_SIZE != infoLst.size()) {
            continue;
        }
        //usb连接日志信息结构
        UsbConnectionInfo info;
        info.sName = infoLst.at(0);
        info.sDatetime = infoLst.at(1);
        info.sStatus = infoLst.at(2);
        info.sSerial = infoLst.at(3);
        info.sVendorID = infoLst.at(4);
        info.sProductID = infoLst.at(5);
        info.sParentSize = infoLst.at(6);
        info.sSize = infoLst.at(7);

        m_usbConnectionInfoLst.append(info);
    }
}

void UsbConnectionWork::readConnectionWhiteLst()
{
    //name--time--isbBlocked--serial--vendorID--productID--sParentSize--sSize
    QString whiteListAllStr = m_monitorNetFlowInvokerInter->Invoke("GetWhiteList", {}, BlockMode::BLOCK)
                                  .arguments()
                                  .first()
                                  .toString();

    QStringList whiteListStrLine = whiteListAllStr.split("\n");
    for (int i = 0; i < whiteListStrLine.size(); i++) {
        QStringList infoLst = whiteListStrLine.at(i).split(",");
        if (USB_INFO_SIZE != infoLst.size()) {
            continue;
        }
        //usb连接信息结构
        UsbConnectionInfo info;
        info.sName = infoLst.at(0);
        info.sDatetime = infoLst.at(1);
        info.sStatus = infoLst.at(2);
        info.sSerial = infoLst.at(3);
        info.sVendorID = infoLst.at(4);
        info.sProductID = infoLst.at(5);
        info.sParentSize = infoLst.at(6);
        info.sSize = infoLst.at(7);

        m_usbConnectionWhiteLst.append(info);
    }
}

void UsbConnectionWork::updateNameInWhiteLst(QString sName, QString sSerial, QString sVendorID, QString sProductID)
{
    if (sName.isEmpty()) {
        return;
    }

    QList<UsbConnectionInfo> lastUsbWhiteList = m_usbConnectionWhiteLst;
    m_usbConnectionWhiteLst.clear();

    for (UsbConnectionInfo info : lastUsbWhiteList) {
        if (sSerial == info.sSerial && sVendorID == info.sVendorID && sProductID == info.sProductID) {
            info.sName = sName;
        }
        m_usbConnectionWhiteLst.append(info);
    }
}

void UsbConnectionWork::onReceiveUsbInfo(const QString &sInfos)
{
    //解析信息字符串
    QStringList infoStrLst = sInfos.trimmed().split(",");
    if (USB_INFO_SIZE != infoStrLst.size()) {
        return;
    }
    if (infoStrLst.at(1).isEmpty()) {
        return;
    }

    UsbConnectionInfo info;
    info.sName = infoStrLst.at(0);
    info.sDatetime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    info.sStatus = infoStrLst.at(2);
    info.sSerial = infoStrLst.at(3);
    info.sVendorID = infoStrLst.at(4);
    info.sProductID = infoStrLst.at(5);
    info.sParentSize = infoStrLst.at(6);
    info.sSize = infoStrLst.at(7);

    updateNameInWhiteLst(info.sName, info.sSerial, info.sVendorID, info.sProductID);

    //记录开关是否打开
    if (!m_isbSaveRecord) {
        return;
    }

    m_usbConnectionInfoLst.append(info);
    //根据当前时间清理连接记录
    cleanUsbConnectionLog();
    Q_EMIT sendAppendUsbConnectionLogInfo(info);
}

void UsbConnectionWork::clearUsbConnectionLog()
{
    m_usbConnectionInfoLst.clear();
}

void UsbConnectionWork::clearSrvUsbConnectionLog()
{
    // 使用服务清空日志
    m_monitorNetFlowInvokerInter->Invoke("ClearUsbConnectionLog", {}, BlockMode::BLOCK);
}

void UsbConnectionWork::cleanUsbConnectionLog()
{
    QList<UsbConnectionInfo> usbConnectionInfoLstTmp = m_usbConnectionInfoLst;
    clearUsbConnectionLog();
    QDateTime limitTime = QDateTime::currentDateTime().addDays(-30);
    for (UsbConnectionInfo info : usbConnectionInfoLstTmp) {
        QDateTime infoTime = QDateTime::fromString(info.sDatetime, "yyyy-MM-dd hh:mm:ss");
        //只记录30天以内的连接数据
        if (limitTime < infoTime) {
            m_usbConnectionInfoLst.append(info);
        }
    }
}

bool UsbConnectionWork::isUsbDeviceInWhiteList(const QString &serial, const QString &vendor, const QString &product)
{
    bool ret = false;
    for (UsbConnectionInfo info : m_usbConnectionWhiteLst) {
        if (serial == info.sSerial && vendor == info.sVendorID && product == info.sProductID) {
            ret = true;
            break;
        }
    }
    return ret;
}

void UsbConnectionWork::appendToUsbConnectionWhiteLst(UsbConnectionInfo info)
{
    //如果id为空，提示添加失败
    if (info.sSerial.isEmpty()) {
        qDebug() << "add to white list failed";
        return;
    }

    m_usbConnectionWhiteLst.append(info);

    //修改后更新服务胡白名单
    QString sDatetime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QVariantList args = {info.sName, sDatetime, info.sStatus, info.sSerial,
                         info.sVendorID, info.sProductID, info.sParentSize, info.sSize};
    m_monitorNetFlowInvokerInter->Invoke("WriteConnectionWhiteLst", args, BlockMode::BLOCK);
}

void UsbConnectionWork::removeFromUsbConnectionWhiteLst(UsbConnectionInfo info)
{
    //如果id为空，提示添加失败
    if (info.sSerial.isEmpty() || info.sVendorID.isEmpty() || info.sProductID.isEmpty()) {
        return;
    }

    for (int i = 0; i < m_usbConnectionWhiteLst.count(); i++) {
        // 修改问题: 在removeFromUsbConnectionWhiteLst中,将白名单列表写成连接记录列表,导致读取列表数据越界
        UsbConnectionInfo infoTmp = m_usbConnectionWhiteLst.at(i);
        if (info.sSerial == infoTmp.sSerial && info.sVendorID == infoTmp.sVendorID && info.sProductID == infoTmp.sProductID) {
            m_usbConnectionWhiteLst.removeAt(i);
            i--;
        }
    }

    //修改后更新服务的白名单
    QVariantList args = {info.sName, info.sSerial, info.sVendorID, info.sProductID};
    m_monitorNetFlowInvokerInter->Invoke("RemoveFromWhiteLst", args, BlockMode::BLOCK);
}

void UsbConnectionWork::changeIsbSaveRecord()
{
    m_isbSaveRecord = !m_isbSaveRecord;
    m_monitorNetFlowInvokerInter->Invoke("ChangeIsbSaveRecord", {m_isbSaveRecord}, BlockMode::BLOCK);
    // 通知去改变连接记录开关
    Q_EMIT notifyChangeIsbSaveRecord();
}

void UsbConnectionWork::selectLimitationModel(const QString &modelStr)
{
    m_limitationModel = modelStr;

    m_monitorNetFlowInvokerInter->Invoke("SelectLimitationModel", {modelStr}, BlockMode::BLOCK);
    // 通知去改变usb限制模式
    Q_EMIT notifyChangeLimitationModel(modelStr);
}
