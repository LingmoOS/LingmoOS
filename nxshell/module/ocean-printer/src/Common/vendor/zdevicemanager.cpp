// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zdevicemanager.h"
#include "cupsattrnames.h"
#include "cupsconnection.h"
#include "qtconvert.h"
#include "common.h"
#include "config.h"
#include "cupsconnectionfactory.h"

#include <QProcess>
#include <QRegularExpression>
#include <QStringList>
#include <QTcpSocket>
#include <QUrl>
#include <QLibrary>

#include <libsmbclient.h>

#include <limits.h>
#include <stdlib.h>

typedef SMBCCTX *(*pfunc_smbc_new_context)(void);
typedef void (*pfunc_smbc_setFunctionAuthDataWithContext)(SMBCCTX *c, smbc_get_auth_data_with_context_fn fn);
typedef SMBCCTX *(*pfunc_smbc_init_context)(SMBCCTX *context);
typedef smbc_opendir_fn(*pfunc_smbc_getFunctionOpendir)(SMBCCTX *c);
typedef smbc_readdir_fn(*pfunc_smbc_getFunctionReaddir)(SMBCCTX *c);
typedef smbc_close_fn(*pfunc_smbc_getFunctionClose)(SMBCCTX *c);
typedef int (*pfunc_smbc_free_context)(SMBCCTX *context, int shutdown_ctx);

static pfunc_smbc_new_context p_smbc_new_context = NULL;
static pfunc_smbc_setFunctionAuthDataWithContext p_smbc_setFunctionAuthDataWithContext = NULL;
static pfunc_smbc_init_context p_smbc_init_context = NULL;
static pfunc_smbc_getFunctionOpendir p_smbc_getFunctionOpendir = NULL;
static pfunc_smbc_getFunctionReaddir p_smbc_getFunctionReaddir = NULL;
static pfunc_smbc_getFunctionClose p_smbc_getFunctionClose = NULL;
static pfunc_smbc_free_context p_smbc_free_context = NULL;

static bool g_smbAuth;
static QString g_smbworkgroup;
static QString g_smbuser;
static QString g_smbpassword;

#define ERR_SocketBase 1000
#define SOCKET_Timeout 3000

static TBackendSchemes g_backendSchemes[] = {{"usb", CUPS_EXCLUDE_NONE},
    {"hp", CUPS_EXCLUDE_NONE},
    {"snmp", CUPS_EXCLUDE_NONE},
    {"smfpnetdiscovery", CUPS_EXCLUDE_NONE},
    {CUPS_INCLUDE_ALL, "cups-brf,dcp,parallel,serial,hpfax"}
};

static void smb_auth_func(SMBCCTX *c,
                          const char *srv,
                          const char *shr,
                          char *wg, int wglen,
                          char *un, int unlen,
                          char *pw, int pwlen)
{
    UNUSED(c);
    UNUSED(srv);
    UNUSED(shr);
    if (g_smbworkgroup.isEmpty()) {
        g_smbworkgroup = wg;
        strncpy(wg, "WORKGROUP", wglen);
    } else
        strncpy(wg, g_smbworkgroup.toUtf8().constData(), wglen);

    if (g_smbuser.isEmpty()) {
        g_smbuser = un;
        /* if user is empty, wu use nobody instead,
         * otherwise smbspool will use kerberos authentication */
        strncpy(un, "nobody", unlen);
    } else
        strncpy(un, g_smbuser.toUtf8().constData(), unlen);

    if (g_smbpassword.isEmpty())
        strncpy(pw, " ", pwlen);
    else
        strncpy(pw, g_smbpassword.toUtf8().constData(), pwlen);
}

RefreshDevicesTask::RefreshDevicesTask(int id, QObject *parent)
    : TaskInterface(id, parent)
{
}

QList<TDeviceInfo> RefreshDevicesTask::getResult()
{
    QMutexLocker locker(&m_mutex);

    return m_devices;
}

int RefreshDevicesTask::addDevice(const TDeviceInfo &dev)
{
    {
        QMutexLocker locker(&m_mutex);

        m_devices.append(dev);
    }

    emit signalStatus(m_iTaskId, TStat_Update);

    return 0;
}

void RefreshDevicesTask::clearDevices()
{
    QMutexLocker locker(&m_mutex);

    m_devices.clear();
}

RefreshDevicesByBackendTask::RefreshDevicesByBackendTask(TBackendSchemes *sechemes, int id, QObject *parent)
    : RefreshDevicesTask(id, parent)
    , m_sechemes(sechemes)
{
}

int RefreshDevicesByBackendTask::mergeDevice(TDeviceInfo &device, const char *backend)
{
    QMutexLocker locker(&m_mutex);

    QString uri = device.uriList[0];
    //排除重复的URI
    for (auto item : m_devices) {
        if (item.uriList.contains(uri)) {
            qCInfo(COMMONMOUDLE) << "remove same uri";
            return -1;
        }
    }

    //合并hplip和usb后端发现的同一台打印机
    if (uri.startsWith("usb:") || uri.startsWith("hp:")) {
        bool isHP = uri.startsWith("hp");
        QRegularExpression re("serial=([^&]*)");
        QRegularExpressionMatch match = re.match(uri);
        if (match.hasMatch()) {
            QString serial = match.captured(1);
            device.serial = serial;
            for (auto &item : m_devices)
                //只合并不同后端发现的uri，相同后端发现的URI应该对应不同设备，比如打印机和传真
                if (!device.strClass.compare(item.strClass) && (item.uriList[0].startsWith("hp:") != isHP) && !serial.compare(item.serial, Qt::CaseInsensitive)) {
                    item.uriList << uri;
                    qCInfo(COMMONMOUDLE) << "merge hp uri";
                    emit signalStatus(m_iTaskId, TStat_Update);
                    return 1;
                }
        }
    }

    //排除三星后端找到重复设备的情况
    if (backend && 0 == strcmp(backend, "smfpnetdiscovery")) {
        for (auto &item : m_devices) {
            if (item.strInfo == device.strInfo) {
                if (item.uriList[0].startsWith("ipp:")) {
                    item.uriList.clear();
                    item.uriList = device.uriList;
                    qCInfo(COMMONMOUDLE) << item.strInfo + ":Uri has been changed";
                    emit signalStatus(m_iTaskId, TStat_Update);
                }
                qCInfo(COMMONMOUDLE) << "remove same device use samsung_schemes";
                return 1;
            }
        }
    }

    return 0;
}

int RefreshDevicesByBackendTask::addDevices(const map<string, map<string, string>> &devs, const char *backend)
{
    map<string, map<string, string>>::const_iterator itmap;

    for (itmap = devs.begin(); itmap != devs.end(); itmap++) {
        TDeviceInfo info;
        QString uri = STQ(itmap->first);
        map<string, string> infomap = itmap->second;

        if (!uri.contains(':'))
            continue;

        dumpStdMapValue(infomap);

        QRegularExpression re_ipv6("([\\da-fA-F]{1,4}:){6}((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)"
                                   "|::([\\da-fA-F]{1,4}:){0,4}((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)"
                                   "|([\\da-fA-F]{1,4}:):([\\da-fA-F]{1,4}:){0,3}((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d"
                                   "|[01]?\\d\\d?)|([\\da-fA-F]{1,4}:){2}:([\\da-fA-F]{1,4}:){0,2}((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]"
                                   "|2[0-4]\\d|[01]?\\d\\d?)|([\\da-fA-F]{1,4}:){3}:([\\da-fA-F]{1,4}:){0,1}((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]"
                                   "|2[0-4]\\d|[01]?\\d\\d?)|([\\da-fA-F]{1,4}:){4}:((25[0-5]|2[0-4]\\d|[01]?\\d\\d?)\\.){3}(25[0-5]|2[0-4]\\d|[01]?\\d\\d?)"
                                   "|([\\da-fA-F]{1,4}:){7}[\\da-fA-F]{1,4}|:((:[\\da-fA-F]{1,4}){1,6}|:)|[\\da-fA-F]{1,4}:((:[\\da-fA-F]{1,4}){1,5}|:)"
                                   "|([\\da-fA-F]{1,4}:){2}((:[\\da-fA-F]{1,4}){1,4}|:)|([\\da-fA-F]{1,4}:){3}((:[\\da-fA-F]{1,4}){1,3}|:)"
                                   "|([\\da-fA-F]{1,4}:){4}((:[\\da-fA-F]{1,4}){1,2}|:)|([\\da-fA-F]{1,4}:){5}:([\\da-fA-F]{1,4})?|([\\da-fA-F]{1,4}:){6}:");
        if (re_ipv6.match(uri).hasMatch()) {
            qCInfo(COMMONMOUDLE) << "Unspport ipv6 uri";
            continue;
        }

        info.uriList << uri;
        info.strClass = attrValueToQString(infomap[CUPS_DEV_CLASS]);
        info.strInfo = attrValueToQString(infomap[CUPS_DEV_INFO]);
        info.strMakeAndModel = attrValueToQString(infomap[CUPS_DEV_MAKE_MODE]);
        info.strDeviceId = attrValueToQString(infomap[CUPS_DEV_ID]);
        info.strLocation = attrValueToQString(infomap[CUPS_DEV_LOCATION]);
        info.iType = InfoFrom_Detect;
        info.strName = info.strInfo;
        /* 过滤dnssd后端发现的打印 避免添加到本地后信息变更导致不可用 */
        if (uri.startsWith("dnssd://")) {
            qCDebug(COMMONMOUDLE) << QString("Does not support non-cups shared dnssd protocol printers,uri=%1").arg(uri);
            continue;
        }
        if (uri.startsWith("dnssd://") && !info.strName.isEmpty()) {
            info.strName = info.strName.split("@").first().trimmed();
        }

        if (0 != mergeDevice(info, backend))
            continue;

        qCDebug(COMMONMOUDLE) << QString("Add printer %1, by:%2").arg(info.toString()).arg(backend ? backend : "other");
        addDevice(info);
    }

    return 0;
}

int RefreshDevicesByBackendTask::doWork()
{
    int sechCount = 1;
    int snmpCount = 0;

    //传入为空的时候用默认的规则查找
    if (!m_sechemes) {
        m_sechemes = g_backendSchemes;
        sechCount = sizeof(g_backendSchemes) / sizeof(g_backendSchemes[0]);
    }

    clearDevices();

    for (int i = 0; i < sechCount; i++) {
        const char *inSech = m_sechemes[i].includeSchemes;
        vector<string> inSechemes, exSechemes;
        map<string, map<string, string>> devs;

        //snmp找到设备的情况不用三星的后端查找设备
        if (snmpCount > 0 && inSech
                && 0 == strcmp(inSech, "smfpnetdiscovery")) {
            continue;
        }

        int lastPrinterCount = getResult().count();
        if (inSech == CUPS_INCLUDE_ALL) {
            QStringList exlist = QString(m_sechemes[i].excludeSchemes).split(",");

            //CUPS_INCLUDE_ALL的情况排除之前已经查找的后端
            for (int j = 0; j < i; j++) {
                exlist.append(m_sechemes[j].includeSchemes);
            }

            exSechemes = qStringListStdVector(exlist);
            qCDebug(COMMONMOUDLE) << "Get devices by all other backends: " << exlist;
        } else {
            inSechemes.push_back(inSech);
            qCDebug(COMMONMOUDLE) << "Get devices by" << inSech;
        }

        try {
            auto conPtr = CupsConnectionFactory::createConnectionBySettings();
            if (conPtr)
                devs = conPtr->getDevices(&exSechemes, &inSechemes, 0, CUPS_TIMEOUT_DEFAULT);
        } catch (const std::exception &ex) {
            qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
            continue;
        };

        if (m_bQuit)
            return 0;

        addDevices(devs, inSech);

        if (inSech && 0 == strcmp(inSech, "snmp"))
            snmpCount = getResult().count() - lastPrinterCount;
    }

    qCInfo(COMMONMOUDLE) << QString("Got %1 devices").arg(getResult().count());

    return 0;
}

RefreshDevicesByHostTask::RefreshDevicesByHostTask(const QString &strHost, int id, QObject *parent)
    : RefreshDevicesTask(id, parent)
{
    m_strHost = strHost;
}

int RefreshDevicesByHostTask::probe_snmp(const QString &strHost)
{
    qCDebug(COMMONMOUDLE) << strHost;
    QString strRet, strErr;
    int iRet = shellCmd(QString("/usr/lib/cups/backend/snmp ") + strHost, strRet, strErr);
    if (0 == iRet) {
        QStringList retList = strRet.split("\n");
        foreach (QString str, retList) {
            if (str.isEmpty())
                continue;

            qCDebug(COMMONMOUDLE) << "Got snmp " << str;
            QStringList list = splitStdoutString(str);
            if (list.count() < 4)
                return -2;

            TDeviceInfo info;
            info.iType = InfoFrom_Host;
            info.strClass = list[0];
            info.uriList << list[1];
            info.strName = info.strMakeAndModel = list[2];
            info.strInfo = list[3];
            if (list.count() > 4)
                info.strDeviceId = list[4];
            if (list.count() > 5)
                info.strLocation = list[5];

            addDevice(info);
        }
    }

    return iRet;
}

int RefreshDevicesByHostTask::probe_hplip(const QString &strHost)
{
    QString strRet;
    QString strErr;
    int iRet = shellCmd(QString("hp-makeuri -c ") + strHost, strRet, strErr);
    strRet = strRet.trimmed();
    if (0 == iRet && strRet.contains(":")) {
        //TODO get uri and info
        TDeviceInfo info;
        info.uriList << strRet;
        info.iType = InfoFrom_Host;
        addDevice(info);
    }

    return iRet;
}

int RefreshDevicesByHostTask::probe_jetdirect(const QString &strHost)
{
    qCDebug(COMMONMOUDLE) << "probe_jetdirect" << strHost;
    QTcpSocket socket;
    socket.connectToHost(strHost, 9100);
    if (socket.waitForConnected(SOCKET_Timeout)) {
        TDeviceInfo info;
        info.uriList << QString("socket://%1:%2").arg(strHost).arg(9100);
        info.iType = InfoFrom_Host;
        qCDebug(COMMONMOUDLE) << info.uriList;
        addDevice(info);
        return 0;
    }

    qCDebug(COMMONMOUDLE) << QString("Connect appsocket %1 failed, err: (%2) %3").arg(strHost).arg(socket.error()).arg(socket.errorString());
    return -1;
}

int RefreshDevicesByHostTask::probe_ipp(const QString &strHost)
{
    map<string, map<string, string>> printersMap;
    map<string, map<string, string>>::iterator itPrinters;
    /*连接指定ip的cups服务器，不是全局的默认ip*/
    auto conPtr = CupsConnectionFactory::createConnection(strHost, 0, 0);
    if (conPtr) {
        try {
            printersMap = conPtr->getPrinters();
        } catch (const std::runtime_error &e) {
            qCWarning(COMMONMOUDLE) << "runtime error:" << e.what();
            return -1;
        }
    } else {
        return -1;
    }

    for (itPrinters = printersMap.begin(); itPrinters != printersMap.end(); itPrinters++) {
        TDeviceInfo info;
        map<string, string> infomap = itPrinters->second;

        dumpStdMapValue(infomap);
        info.strName = STQ(itPrinters->first);
        info.strInfo = attrValueToQString(infomap[CUPS_OP_INFO]);
        info.strLocation = strHost;
        info.strMakeAndModel = attrValueToQString(infomap[CUPS_OP_MAKE_MODEL]);
        info.uriList << attrValueToQString(infomap[CUPS_OP_URI_SUP]);

        addDevice(info);
    }

    return 0;
}

#define LPD_MAX 1
int RefreshDevicesByHostTask::probe_lpd(const QString &strHost)
{
    qCDebug(COMMONMOUDLE) << "probe_lpd" << strHost;
    QTcpSocket socket;
    socket.connectToHost(strHost, 515);
    if (socket.waitForConnected(SOCKET_Timeout)) {
        TDeviceInfo info;
        info.iType = InfoFrom_Host;
        info.uriList << QString("lpd://%1/%2").arg(strHost).arg("Unknown");
        addDevice(info);
        return 0;
    }

    qCDebug(COMMONMOUDLE) << QString("Connect appsocket %1 failed, err: (%2) %3").arg(strHost).arg(socket.error()).arg(socket.errorString());
    return -1;
}

int RefreshDevicesByHostTask::probe_smb(const QString &strHost)
{
    qCDebug(COMMONMOUDLE) << "probe_smb" << strHost;

    if (!p_smbc_init_context) {
        QLibrary smbclient("libsmbclient", 0);
        p_smbc_new_context = (pfunc_smbc_new_context)smbclient.resolve("smbc_new_context");
        p_smbc_setFunctionAuthDataWithContext = (pfunc_smbc_setFunctionAuthDataWithContext)smbclient.resolve("smbc_setFunctionAuthDataWithContext");
        p_smbc_init_context = (pfunc_smbc_init_context)smbclient.resolve("smbc_init_context");
        p_smbc_getFunctionOpendir = (pfunc_smbc_getFunctionOpendir)smbclient.resolve("smbc_getFunctionOpendir");
        p_smbc_getFunctionReaddir = (pfunc_smbc_getFunctionReaddir)smbclient.resolve("smbc_getFunctionReaddir");
        p_smbc_getFunctionClose = (pfunc_smbc_getFunctionClose)smbclient.resolve("smbc_getFunctionClose");
        p_smbc_free_context = (pfunc_smbc_free_context)smbclient.resolve("smbc_free_context");

        if (!p_smbc_new_context || !p_smbc_setFunctionAuthDataWithContext ||
                !p_smbc_init_context || !p_smbc_getFunctionOpendir || !p_smbc_getFunctionReaddir ||
                !p_smbc_getFunctionClose || !p_smbc_free_context) {
            qCWarning(COMMONMOUDLE) << "load libsmbclient error.";
            return -1;
        }
    }

    int ret = 0;
    int try_again = 0;
    QString uri = "smb://";
    if (!strHost.isEmpty())
        uri += QUrl(strHost).toEncoded();
    if (!uri.endsWith('/'))
        uri += '/';
    QByteArray uri_utf8 = uri.toUtf8();

    SMBCCTX *ctx = nullptr;
    SMBCFILE *fd = nullptr;
    if ((ctx = p_smbc_new_context()) == nullptr) {
        ret = -1;
        goto done;
    }

    p_smbc_setFunctionAuthDataWithContext(ctx, smb_auth_func);

    if (p_smbc_init_context(ctx) == nullptr) {
        ret = -2;
        goto done;
    }

    g_smbAuth = false;
    fd = p_smbc_getFunctionOpendir(ctx)(ctx, uri_utf8.constData());
    while (!fd) {
        int last = try_again;
        qCDebug(COMMONMOUDLE) << "error: " << errno;
        //if (errno != EACCES && errno != EPERM) {
        //    qDebug() << errno;
        //    ret = -3;
        //    goto done;
        //}
        g_smbAuth = true;
        emit signalSmbPassWord(try_again, strHost, g_smbworkgroup, g_smbuser, g_smbpassword);
        if (try_again <= last)
            goto done;

        fd = p_smbc_getFunctionOpendir(ctx)(ctx, uri_utf8.constData());
    }

    /* insert workgroup after smb:// */
    if (!g_smbworkgroup.isEmpty())
        uri.insert(6, QUrl(g_smbworkgroup).toEncoded() + '/');

    struct smbc_dirent *dirent;
    while ((dirent = p_smbc_getFunctionReaddir(ctx)(ctx, fd)) != nullptr) {
        if (dirent->smbc_type != SMBC_PRINTER_SHARE)
            continue;

        TDeviceInfo info;
        QUrl url(uri + QUrl(dirent->name).toEncoded());

        if (g_smbAuth) {
            url.setUserName(g_smbuser.isEmpty() ? "nobody" : g_smbuser);
            url.setPassword(g_smbpassword);
        }

        info.uriList << url.toEncoded();

        info.strMakeAndModel = dirent->comment;
        info.strName = info.strInfo = info.strMakeAndModel;
        info.iType = InfoFrom_Host;
        addDevice(info);
    }

done:
    if (fd)
        p_smbc_getFunctionClose(ctx)(ctx, fd);
    if (ctx)
        p_smbc_free_context(ctx, 1);
    qCDebug(COMMONMOUDLE) << "probe_smb ret: " << ret;
    return ret;
}

int RefreshDevicesByHostTask::doWork()
{
    clearDevices();

    m_strLastErr = reslovedHost(m_strHost);
    if (!m_strLastErr.isEmpty()) {
        return -1;
    }

    probe_snmp(m_strHost);
    if (m_bQuit)
        return 0;

    //如果snmp 找到设备则不用查找socket、ipp、lpd
    if (getResult().size() <= 0) {
        probe_jetdirect(m_strHost);
        if (m_bQuit)
            return 0;

        probe_ipp(m_strHost);
        if (m_bQuit)
            return 0;

        probe_lpd(m_strHost);
        if (m_bQuit)
            return 0;
    }

    probe_hplip(m_strHost);
    if (m_bQuit)
        return 0;

    /*去掉smb协议的打印机探测，网络不通的情况下必然会出现，但是用户一般都不知道samba协议的用法，以为需要登陆才能查找打印机
     *后面考虑把samba协议挪到手动输入uri中，检测smb协议的前缀然后根据是否需要认证选择性弹出对话框
    */
    return 0;
}
