// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZDEVICEMANAGER_H
#define ZDEVICEMANAGER_H

#include "ztaskinterface.h"

#include <QString>
#include <QMap>
#include <QObject>
#include <QList>
#include <QVariant>
#include <QMutexLocker>

#include <stdlib.h>
#include <string>
#include <map>
using namespace std;

enum {
    InfoFrom_Invalid = -1,
    InfoFrom_Detect = 0, /*准确的打印机信息，通过探测发现的*/
    InfoFrom_Host, /*通过host猜测的打印机信息*/
    InfoFrom_Manual /*用户手动填写的打印机信息*/
};

typedef struct tagDeviceInfo {
    tagDeviceInfo()
        : iType(InfoFrom_Invalid)
    {
    }

    QString toString() const
    {
        return QString("uri: %1,class:%2,info:%3,name:%4,makemodel:%5,id:%6,location:%7,type:%8,serial:%9") \
                .arg(uriList.join(";")).arg(strClass).arg(strInfo).arg(strName).arg(strMakeAndModel) \
                .arg(strDeviceId).arg(strLocation).arg(iType).arg(serial);
    }

    QStringList uriList; //uri
    QString strClass; //打印机类型，network网络共享打印机，direct直连打印机，file云打印
    QString strInfo; //打印机描述
    QString strName; //打印机名字，用户在打印机设置界面设置的名字
    QString strMakeAndModel; //打印机厂商和型号
    QString strDeviceId; //打印机IEEE 1284 ID
    QString strLocation; //location
    int iType; //打印机信息的来源
    QString serial; //打印机序列号
} TDeviceInfo;

Q_DECLARE_METATYPE(TDeviceInfo)

class RefreshDevicesTask : public TaskInterface
{
    Q_OBJECT

public:
    RefreshDevicesTask(int id, QObject *parent = nullptr);

    QList<TDeviceInfo> getResult();

protected:
    int addDevice(const TDeviceInfo &dev);
    void clearDevices();

private:
    QList<TDeviceInfo> m_devices;
    QMutex m_mutex;

    friend class RefreshDevicesByBackendTask;
};

typedef struct tagBackendSchemes {
    const char *includeSchemes;
    const char *excludeSchemes;
} TBackendSchemes;

class RefreshDevicesByBackendTask : public RefreshDevicesTask
{
    Q_OBJECT

public:
    //支持传入查找规则，如果sechemes不为空，则认为传入一个TBackendSchemes结构体
    RefreshDevicesByBackendTask(TBackendSchemes *sechemes = nullptr, int id = TASK_RefreshKnownDev, QObject *parent = nullptr);

protected:
    int doWork();

    int mergeDevice(TDeviceInfo &device, const char *backend);
    int addDevices(const map<string, map<string, string>> &devs, const char *backend);

private:
    TBackendSchemes *m_sechemes;
};

class RefreshDevicesByHostTask : public RefreshDevicesTask
{
    Q_OBJECT

public:
    RefreshDevicesByHostTask(const QString &strHost, int id = TASK_RefreshNetDev, QObject *parent = nullptr);

signals:
    //必须以Qt::BlockingQueuedConnection方式connect，保证阻塞住emit线程
    //如果确认执行输入密码操作将ret+1,否则取消
    //槽函数返回之前需要设置password、group、user传回给Samba函数处理
    void signalSmbPassWord(int &ret, QString host, QString &group, QString &user, QString &password);

protected:
    int doWork();

    int probe_hplip(const QString &strHost);
    int probe_jetdirect(const QString &strHost);
    int probe_ipp(const QString &strHost);
    int probe_lpd(const QString &strHost);
    int probe_smb(const QString &strHost);
    int probe_snmp(const QString &strHost);

private:
    QString m_strHost;
};

#endif // ZDEVICEMANAGER_H
