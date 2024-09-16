// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPRINTMANAGER_H
#define DPRINTMANAGER_H

#include "cupsconnection.h"
#include "ddestination.h"
#include "dprintertanslator.h"

#include <QStringList>
#include <QMap>
#include <QObject>
#include <QLocale>

using namespace std;

class DPrinterManager : public QObject
{
    Q_OBJECT

public:
    static DPrinterManager *getInstance();
    ~DPrinterManager();

public:

    //set allowed user
    void setAllowedUsers(const QString &strPrinterName, const QVector<QString> strUsers);
    void updateDestinationList();
    // 获取打印机名称列表
    QStringList getPrintersList();
    // 根据打印机名称获取基础信息
    QStringList getPrinterBaseInfoByName(QString printerName);
    // 根据名称删除打印机
    bool deletePrinterByName(QString PrinterName);
    // 设置共享
    void setPrinterShared(QString printerName, int shared);
    bool isPrinterShared(QString printerName);
    // 设置启用
    void setPrinterEnabled(QString printerName, bool enabled);
    bool isPrinterEnabled(QString printerName);
    // 设置接受任务
    void setPrinterAcceptJob(QString printerName, bool enabled);
    bool isPrinterAcceptJob(QString printerName);

    // 设置默认打印机
    void setPrinterDefault(QString printerName);
    // 添加打印机
    bool addPrinter(const QString &printer, const QString &info, const QString &location, const QString &device, const QString &ppdfile);

    // 判断打印机是否是默认打印机
    bool isDefaultPrinter(QString PrinterName);
    //初始化语方翻译
    void initLanguageTrans();
    //文件翻译
    QString translateLocal(const QString &strContext, const QString &strKey, const QString &strDefault);
    DDestination *getDestinationByName(const QString &strName);
    // 判断存在所有未完成的打印任务
    bool hasUnfinishedJob();
    // 判断该打印机是否存在未完成的任务
    bool hasUnfinishedJob(const QString &printer);
    // 判断存在完成的打印任务
    bool hasFinishedJob();

    //服务器设置接口
    void enableDebugLogging(bool enabled);
    void enableRemoteAdmin(bool enabled);
    void enableRemoteAny(bool enabled);
    void enableSharePrinters(bool enabled);
    void enableUserCancelAny(bool enabled);

    bool isDebugLoggingEnabled() const;
    bool isRemoteAdminEnabled() const;
    bool isRemoteAnyEnabled() const;
    bool isSharePrintersEnabled() const;
    bool isUserCancelAnyEnabled() const;
    /**
    * @projectName   Printer
    * @brief         commit为真正的设置服务器参数接口，上述其他接口只是暂存参数
    * @author        liurui
    * @date          2019-11-09
    */
    bool updateServerSetting();
    void commit(const map<string, string> &options);
    bool hasSamePrinter(const QString &printer);
    QString validataName(const QString &oldPrinterName);

private:
    DPrinterManager();
    Q_DISABLE_COPY(DPrinterManager)

private:
    void clearDestinationList();

private:
    static DPrinterManager *m_self;

    QMap<QString, DDestination *> m_mapDests;

    DPrinterTanslator m_translator;
    //服务器设置
    ServerSettings m_pServerSettings;
};


#endif // DPRINTMANAGER_H
