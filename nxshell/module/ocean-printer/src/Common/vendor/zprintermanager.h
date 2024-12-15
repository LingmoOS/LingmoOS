// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPRINTERMANAGER_H
#define DPRINTERMANAGER_H

#include "zdevicemanager.h"
#include "cupsconnection.h"

#include <QString>
#include <QMap>
#include <QObject>
#include <QList>
#include <QVariant>

#include <cups/cups.h>

//获取系统已经添加的打印机信息
int GetSystemPrinters();

//尝试通过uri获取打印机信息
int GetPrinterInfo(TDeviceInfo &printer);

//添加系统打印机
int AddSystemPrinter(TDeviceInfo printer, QVariantMap solution);

//移除系统打印机
int RemoveSystemPrinter(QString printername);

//复制系统打印机
int DuplicateSystemPrinter(TDeviceInfo printer, QString newname);

//重命名系统打印机
int RenameSystemPrinter(TDeviceInfo printer, QString newname);

//设置系统打印机信息
int SetSytemPrinterInfo(TDeviceInfo printer);

//设置系统默认打印机
int SetDefaultPrinter(QString printername);
const char *GetDefaultPrinter(void);

//设置打印机是否共享
int SetPrinterShared(QString printername, bool bShared = true);

//设置打印机释放接受任务
int setPrinterAcceptingJobs(const char *name, bool bAccept = true);

//停用/启用打印机
int setPrinterDisable(const char *name, bool bDisable = true);

class DPrinterManager : public QObject
{
    Q_OBJECT

public:
    static DPrinterManager *getInstance();
    ~DPrinterManager();

public:
    bool InitConnection(const char *host_uri, int port, int encryption);
    Connection *getConnection();

private:
    DPrinterManager();
    Q_DISABLE_COPY(DPrinterManager)

private:
    Connection *m_conn;
};

#define g_cupsConnection DPrinterManager::getInstance()->getConnection()

#endif // DPRINTERMANAGER_H
