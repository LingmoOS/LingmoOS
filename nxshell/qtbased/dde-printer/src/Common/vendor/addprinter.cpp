// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "addprinter.h"
#include "addprinter_p.h"
#include "zdrivermanager.h"
#include "cupsattrnames.h"
#include "ztaskinterface.h"
#include "printerservice.h"
#include "common.h"
#include "cupsconnection.h"
#include "qtconvert.h"
#include "cupsconnectionfactory.h"

#include <QRegularExpression>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QJsonParseError>
#include <QFileInfo>
#include <QDBusInterface>
#include <QNetworkReply>
#include <QDBusReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcessEnvironment>

static QString g_captexec = "/opt/cndrvcups-capt/add";

static QString getPackageVersion(const QString &package)
{
    QString strOut, strErr;
    if (0 == shellCmd(QString("dpkg -l %1").arg(package), strOut, strErr)) {
        QStringList list = strOut.split("\n", QString::SkipEmptyParts);
        strOut = list.isEmpty() ? "" : list.last();
        qCDebug(COMMONMOUDLE) << strOut;
        list = strOut.split(" ", QString::SkipEmptyParts);
        return list.count() > 2 ? list[2] : QString();
    }

    return QString();
}

static bool isCanonCAPTDrv(const QString &ppd_name)
{
    QRegularExpression re("CNCUPS.*CAPT.*\\.ppd");
    QRegularExpressionMatch match = re.match(ppd_name);
    return match.hasMatch();
}

static bool isHplipDrv(const QString &ppd_name)
{
    return (ppd_name.startsWith("drv:///hpcups.drv") || ppd_name.startsWith("drv:///hpijs.drv") || ppd_name.startsWith("lsb/usr/hplip/") ||
            ppd_name.startsWith("hplip:") || ppd_name.startsWith("hplip-data:") || ppd_name.startsWith("hpijs-ppds:") || ppd_name.startsWith("hp/hp-"));
}

static void dbusAutoStart(const QString &filePath)
{
    QDBusInterface interface("com.deepin.printer.manager", "/com/deepin/printer/manager", "com.deepin.printer.manager", QDBusConnection::systemBus());
    interface.call("LaunchAutoStart", filePath);
}

static void checkPPDName(const QString &ppdname)
{
    QString folderPath = "/opt/printer-drivers";
    QDir dir(folderPath);
    if (!dir.exists()) {
        qCDebug(COMMONMOUDLE) << "Folder does not exist: ";
        return;
    }

    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList folders = dir.entryInfoList();

    for (int i = 0; i < folders.size(); ++i) {
        QString folderName = folders.at(i).fileName();
        QString filePath = folderPath + "/" + folderName + "/config/";
        QString ppdPath = filePath +  "ppds";

        QFile file(ppdPath);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.contains(ppdname)) {
                file.close();
                dbusAutoStart(filePath);
                if (folderName == "com.pantum.pantum") {
                    QStringList args;
                    QProcess::startDetached(filePath + "autostart.sh", QStringList() << "-r");
                }
                return;
            }
        }

        file.close();
    }
}

static QDBusInterface *getPackageInterface()
{
    QDBusInterface newinterface
    {
        "org.deepin.dde.Lastore1",
        "/org/deepin/dde/Lastore1",
        "org.deepin.dde.Lastore1.Manager",
        QDBusConnection::systemBus()
    };

    if (newinterface.isValid()) {
        static QDBusInterface interface
        {
            "org.deepin.dde.Lastore1",
            "/org/deepin/dde/Lastore1",
            "org.deepin.dde.Lastore1.Manager",
            QDBusConnection::systemBus()
        };

        return &interface;
    }

    static QDBusInterface interface
    {
        "com.deepin.lastore",
        "/com/deepin/lastore",
        "com.deepin.lastore.Manager",
        QDBusConnection::systemBus()
    };

    return &interface;
}

static QString getDriverFrom(int type)
{
    QString from;
    switch (type) {
        case PPDFrom_File:
            from = "file";
            break;

        case PPDFrom_Server:
            from = "network";
            break;

        case PPDFrom_EveryWhere:
            from = "everywhere";
            break;

        default:
            from = "local";
            break;
    }
    return from;
}

static QString getSearchType(int num)
{
    QString type;
    switch (num) {
        case InfoFrom_Detect:
            type = "auto";
            break;

        case InfoFrom_Host:
            type = "host";
            break;

        case InfoFrom_Manual:
            type = "uri";
            break;

        default:
            type = "unknown";
            break;
    }
    return type;
}

static bool containsIpAddress(const QString &str) {
    QRegularExpression ipRegex("\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b");
    QRegularExpressionMatch match = ipRegex.match(str);
    return match.hasMatch();
}

static int shellCmdLocal(const QString &cmd, QString &out, QString &strErr, int timeout)
{
    QProcess proc;
    QStringList env = QProcess::systemEnvironment();
    env.append("LANG=utf-8");
    proc.setEnvironment(env);
    proc.start(cmd);
    if (proc.waitForFinished(timeout)) {
        out = proc.readAll();
        if (proc.exitCode() != 0 || proc.exitStatus() != QProcess::NormalExit) {
            strErr = QString("err %1, string: %2").arg(proc.exitCode()).arg(QString::fromUtf8(proc.readAllStandardError()));
            qCDebug(COMMONMOUDLE) << cmd;
            qCWarning(COMMONMOUDLE) << "shellCmd exit with err: " << strErr;
            return -1;
        }
    } else {
        qCWarning(COMMONMOUDLE) << "shellCmd timeout";
        return -2;
    }

    return 0;
}

static bool isPackageInstallable(const QString &packagename)
{
    QString strRet, strErr;
    QString srcParts = "Dir::Etc::SourceParts=/etc/apt/sources.list.d";
    QString srcList = "Dir::Etc::SourceList=/etc/deepin/lastore-daemon/sources.list.d/local-driver.list";
    int iRet = shellCmdLocal(QString("apt policy %1 -o %2 -o %3").arg(packagename).arg(srcParts).arg(srcList), strRet, strErr, 5000);
    if (iRet == 0 && !strRet.isEmpty() && strRet.contains("Candidate:", Qt::CaseInsensitive)){
        return true;
    }

    return false;
}

FixHplipBackend::FixHplipBackend(QObject *parent)
    : QObject(parent)
{
    m_deviceTask = nullptr;
    m_installer = nullptr;
}

int FixHplipBackend::startFixed()
{
    if (!m_installer) {
        m_installer = new InstallInterface(this);

        QList<TPackageInfo> packages;
        TPackageInfo info;
        info.packageName = "hplip";
        packages.append(info);
        m_installer->setPackages(packages);

        connect(m_installer, &InstallInterface::signalStatus, this, &FixHplipBackend::slotInstallStatus);
    }

    m_installer->startInstallPackages();
    return 0;
}

void FixHplipBackend::stop()
{
    if (m_installer)
        m_installer->stop();

    if (m_deviceTask)
        m_deviceTask->stop();
}

QString FixHplipBackend::getErrorString()
{
    return m_strError;
}

QString FixHplipBackend::getMatchHplipUri(const QString &strUri)
{
    QString strMatch;

    if (m_deviceTask) {
        QList<TDeviceInfo> devices = m_deviceTask->getResult();

        QRegularExpression re("serial=([^&]*)");
        QRegularExpressionMatch match = re.match(strUri);
        if (match.hasMatch()) {
            QString serial = match.captured(1).toLower();

            for (int i = 0; i < devices.count(); i++) {
                QRegularExpressionMatch devMatch = re.match(devices[i].uriList[0]);
                QString devSerial = devMatch.hasMatch() ? match.captured(1).toLower() : "";

                if (!devSerial.isEmpty() && devSerial == serial) {
                    qCDebug(COMMONMOUDLE) << "getMatchHplipUri";
                    return devices[i].uriList[0];
                }
            }
        }
    }

    return strMatch;
}

void FixHplipBackend::slotDeviceStatus(int id, int status)
{
    Q_UNUSED(id);

    qCInfo(COMMONMOUDLE) << status;

    if (TStat_Suc == status || TStat_Fail == status) {
        if (TStat_Fail == status) {
            m_strError = m_deviceTask->getErrorString();
        }
        emit signalStatus(status);
    }
}

void FixHplipBackend::slotInstallStatus(int status)
{
    qCInfo(COMMONMOUDLE) << status;

    if (TStat_Suc == status) {
        if (nullptr == m_deviceTask) {
            m_hplipBackend.excludeSchemes = CUPS_EXCLUDE_NONE;
            m_hplipBackend.includeSchemes = "hp";
            m_deviceTask = new RefreshDevicesByBackendTask(&m_hplipBackend);
            connect(m_deviceTask, &RefreshDevicesByBackendTask::signalStatus, this, &FixHplipBackend::slotDeviceStatus);
        }

        m_deviceTask->start();
    } else {
        m_strError = m_installer->getErrorString();
        emit signalStatus(TStat_Fail);
    }
}

InstallInterface::InstallInterface(QObject *parent)
    : QObject(parent)
    , m_bQuit(false)
{
}

void InstallInterface::setPackages(const QList<TPackageInfo> &packages)
{
    m_packages = packages;
}

QString InstallInterface::getErrorString()
{
    return m_strErr;
}

void InstallInterface::updateSourceChanged(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    if (m_bQuit)
        return;

    if (arguments.count() == 3) {
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments[1].value<QDBusArgument>());
        for (auto prop = changedProps.begin(); prop != changedProps.end(); ++prop) {
            QString key = prop.key();
            qCDebug(COMMONMOUDLE) << "key : " << key << " Value: " <<  prop.value().toString();
            if (key == "Type") {
                m_strUpdateType = prop.value().toString();
            } else if (key == "Status") {
                m_strUpdateStatus = prop.value().toString();
            }

            if (m_strUpdateType == "update_source" && (m_strUpdateStatus == "end" || m_strUpdateStatus == "succeed")) {
                m_strUpdateType.clear();
                m_strUpdateStatus.clear();
                goto endUpdate;
            } else if (m_strUpdateStatus == "failed") {
                goto endUpdate;
            }
        }
    }
    return;

endUpdate:
    qCDebug(COMMONMOUDLE) << "Disconnect com.deepin.lastore updateSourceChanged";
    QDBusConnection::systemBus().disconnect("com.deepin.lastore",
                                            m_updatePath,
                                            "org.freedesktop.DBus.Properties",
                                            "PropertiesChanged",
                                            this, SLOT(updateSourceChanged(QDBusMessage)));
    sleep(1); // 等待1s为了update后进行安装
    startInstallPackages(false);
    return;
}

void InstallInterface::startInstallPackages(bool status)
{
    /*检查包是否安装，并且校验版本，只有需要安装的时候才调用dbus接口安装
     * 防止驱动已经安装的情况因为apt报错导致添加打印机失败
     * TODO dbus安装没有执行apt update，可能存在更新不成功或者安装失败的问题
     * 目前依赖系统自动执行的apt update
    */
    QDBusInterface *interface = getPackageInterface();
    for (auto package : m_packages) {
        qCInfo(COMMONMOUDLE) << "install package:" << package.toString();
        if (isPackageExists(package.packageName)) {
            QString strVer = getPackageVersion(package.packageName);
            if (!package.packageVer.isEmpty() && strVer != package.packageVer) {
                qCInfo(COMMONMOUDLE) << package.packageName << "need update";
                m_installPackages.append(package.packageName);
            } else {
                qCInfo(COMMONMOUDLE) << package.packageName << "is exists: " << strVer;
                continue;
            }
        } else {
            m_installPackages.append(package.packageName);
            qCInfo(COMMONMOUDLE) << package.packageName << "need install";
        }

        /*hplip-plugin包 mips架构目前不存在，所以忽略无法安装的错误，避免安装打印机流程阻塞*/
        if (!isPackageInstallable(package.packageName) && (!package.packageName.contains("hplip-plugin"))) {
            if (status) {
                QDBusReply<QDBusObjectPath> updatePath = interface->call("UpdateSource");
                m_updatePath = updatePath.value().path();

                if (QDBusConnection::systemBus().connect("com.deepin.lastore",
                                                         m_updatePath,
                                                         "org.freedesktop.DBus.Properties",
                                                         "PropertiesChanged",
                                                         this, SLOT(updateSourceChanged(QDBusMessage)))) {
                    qCInfo(COMMONMOUDLE) << "Start Update";
                    return;
                }
            }

            m_strErr = package.packageName + tr("is invalid");
            emit signalStatus(TStat_Fail);
            return;
        }
    }

    if (m_installPackages.isEmpty()) {
        emit signalStatus(TStat_Suc);
        return;
    }

    QDBusReply<QDBusObjectPath> objPath = interface->call("InstallPackage", "", m_installPackages.join(" "));
    QString extInfo;

    if (objPath.isValid()) {
        m_jobPath = objPath.value().path();
        if (QDBusConnection::systemBus().connect(interface->service(),
                                                 m_jobPath,
                                                 "org.freedesktop.DBus.Properties",
                                                 "PropertiesChanged",
                                                 this, SLOT(propertyChanged(QDBusMessage)))) {
            qCDebug(COMMONMOUDLE) << "Start install " << m_installPackages;
            return;
        } else {
            extInfo = "Connect dbus signal failed";
            qCWarning(COMMONMOUDLE) << "Connect dbus signal failed";
        }
    } else {
        extInfo = objPath.error().message();
        qCWarning(COMMONMOUDLE) << "DBus error: " << objPath.error().message();
    }

    m_strErr = tr("Failed to install the driver by calling dbus interface");
    m_strErr += extInfo;
    emit signalStatus(TStat_Fail);
}

void InstallInterface::stop()
{
    m_bQuit = true;
    QDBusConnection::systemBus().disconnect(getPackageInterface()->service(),
                                            m_jobPath,
                                            "org.freedesktop.DBus.Properties",
                                            "PropertiesChanged",
                                            this, SLOT(propertyChanged(QDBusMessage)));

    QDBusConnection::systemBus().disconnect("com.deepin.lastore",
                                            m_updatePath,
                                            "org.freedesktop.DBus.Properties",
                                            "PropertiesChanged",
                                            this, SLOT(updateSourceChanged(QDBusMessage)));
}

void InstallInterface::propertyChanged(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();

    if (m_bQuit)
        return;

    if (3 == arguments.count()) {
        QString strType, strStatus;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments[1].value<QDBusArgument>());
        for (auto prop = changedProps.begin(); prop != changedProps.end(); ++prop) {
            QString key = prop.key();
            if (key == "Type")
                m_strType = prop.value().toString();
            else if (key == "Status")
                m_strStatus = prop.value().toString();
        }

        qCDebug(COMMONMOUDLE) << m_strType << m_strStatus;
        if (m_strType == "install" && m_strStatus == "succeed") {
            emit signalStatus(TStat_Suc);
            goto done;
        } else if (m_strStatus == "failed") {
            m_strErr = tr("Failed to install %1").arg(m_installPackages.join("  "));
            stop(); // 失败时先disconnect，避免成员析构导致sigsegv
            emit signalStatus(TStat_Fail);
        }

        return;
    }

    m_strErr = tr("Failed to install %1").arg(m_installPackages.join(" "));
    emit signalStatus(TStat_Fail);

done:
    qCDebug(COMMONMOUDLE) << "Disconnect com.deepin.lastore PropertiesChanged";
    QDBusConnection::systemBus().disconnect(getPackageInterface()->service(),
                                            m_jobPath,
                                            "org.freedesktop.DBus.Properties",
                                            "PropertiesChanged",
                                            this, SLOT(propertyChanged(QDBusMessage)));

    return;
}

InstallDriver::InstallDriver(const QMap<QString, QVariant> &solution, QObject *parent)
    : InstallInterface(parent)
{
    m_solution = solution;
}

void InstallDriver::doWork()
{
    qCDebug(COMMONMOUDLE) << "Search driver for" << m_solution;
    QString strPackageName, strPackageVer;
    strPackageName = m_solution[SD_KEY_driver].toString();
    if (!strPackageName.isEmpty()) {
        TPackageInfo info;
        info.packageName = strPackageName;
        info.packageVer = m_solution[SD_KEY_debver].toString();
        m_packages.append(info);
        startInstallPackages();
    } else {
        emit signalStatus(TStat_Fail);
    }
}

void InstallDriver::stop()
{
    InstallInterface::stop();
}

void InstallDriver::slotServerDone(int iCode, const QByteArray &result)
{
    sender()->deleteLater();
    if (m_bQuit)
        return;

    if (iCode != QNetworkReply::NoError) {
        m_strErr = tr("Failed to find the driver solution: %1, error: %2")
                   .arg(m_solution[SD_KEY_sid].toInt())
                   .arg(iCode);
        qCWarning(COMMONMOUDLE) << "Request " << m_solution[SD_KEY_sid] << "failed:" << iCode;
        emit signalStatus(TStat_Fail);
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(result, &err);
    if (doc.isNull()) {
        m_strErr = tr("The solution is invalid");
        qCWarning(COMMONMOUDLE) << "Request " << m_solution[SD_KEY_sid] << " return nullptr";
        emit signalStatus(TStat_Fail);
        return;
    }

    qCDebug(COMMONMOUDLE) << doc.toJson();
    QJsonObject obj = doc.object();
    QJsonArray package_array = obj[SD_KEY_package].toArray();
    QJsonArray ver_array = obj[SD_KEY_ver].toArray();

    for (int i = 0; i < package_array.size(); i++) {
        TPackageInfo info;
        info.packageName = package_array.at(i).toString();
        info.packageVer = ver_array.at(i).isUndefined() ? QString() : ver_array.at(i).toString();
        m_packages.append(info);
    }

    startInstallPackages();
}

AddCanonCAPTPrinter::AddCanonCAPTPrinter(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution, const QString &uri, QObject *parent)
    : AddPrinterTask(printer, solution, uri, parent)
{
    connect(&m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));
}

void AddCanonCAPTPrinter::stop()
{
    AddPrinterTask::stop();

    disconnect(&m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));
    m_proc.kill();
}

int AddCanonCAPTPrinter::addPrinter()
{
    QString ppd_name;
    ppd_name = m_solution[CUPS_PPD_NAME].toString();

    if (!QFile::exists(g_captexec)) {
        qCWarning(COMMONMOUDLE) << g_captexec << "not found";
        return -1;
    }

    if (m_bQuit)
        return -1;

    QString printInfo = "usb:";
    if (containsIpAddress(m_uri)) {
        printInfo = "net:";
        QRegularExpression re("([0-9]{1,3}\\.){3}[0-9]{1,3}");
        QRegularExpressionMatch match = re.match(m_uri);

        if (match.hasMatch()) {
            printInfo += match.captured(0);
        }
    } else {
        printInfo += m_printer.serial;
    }

    int index = ppd_name.lastIndexOf('/');
    if (index != -1) {
        ppd_name = ppd_name.mid(index + 1);
    }

    qCDebug(COMMONMOUDLE) << "ppdname : " << ppd_name << "printInfo: " << printInfo;
    int ret = managerCanonPrinter("add", QStringList {m_printer.strName, ppd_name, printInfo});

    m_proc.setProcessChannelMode(QProcess::ForwardedChannels);
    if (ret != 0) {
        emit m_proc.finished(ret, QProcess::CrashExit);
    } else {
        emit m_proc.finished(ret, QProcess::NormalExit);
    }

    return 1;
}

void AddCanonCAPTPrinter::slotProcessFinished(int iCode, QProcess::ExitStatus exitStatus)
{
    qCInfo(COMMONMOUDLE) << iCode << exitStatus;
    if (exitStatus != QProcess::NormalExit || 0 != iCode) {
        m_strErr = m_proc.readAllStandardError();
        if (iCode == 2) { // 佳能capt打印机已经添加，不允许重复添加
            m_strErr = QObject::tr("The Canon capt printer has been added and is not allowed to be added again.");
        } else if (iCode == -11) {
            m_strErr = "Application path does not match, and installation is not allowed.";
        }

        m_strErr += "Canon capt install fail.";
        m_iStep = STEP_Failed;
    }

    nextStep();
}

DefaultAddPrinter::DefaultAddPrinter(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution, const QString &uri, QObject *parent)
    : AddPrinterTask(printer, solution, uri, parent)
{
    connect(&m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));
}

void DefaultAddPrinter::stop()
{
    AddPrinterTask::stop();

    disconnect(&m_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessFinished(int, QProcess::ExitStatus)));
    m_proc.kill();
}

int DefaultAddPrinter::addPrinter()
{
    QStringList args;
    QString ppd_name;
    int driverType = m_solution[SD_KEY_from].toInt();
    ppd_name = m_solution[CUPS_PPD_NAME].toString();

    if (m_bQuit)
        return -1;

    args << "-p" << m_printer.strName << "-E"
         << "-v" << m_uri;
    if (PPDFrom_EveryWhere == driverType)
        args << "-m"
             << "everywhere";
    else if (driverType == PPDFrom_File)
        args << "-P" << ppd_name;
    else
        args << "-m" << ppd_name;

    if (!m_printer.strInfo.isEmpty())
        args << "-D" << m_printer.strInfo;
    if (!m_printer.strLocation.isEmpty())
        args << "-L" << m_printer.strLocation;

    qCDebug(COMMONMOUDLE) << args;
    m_proc.start("/usr/sbin/lpadmin", args);

    return 1;
}

void DefaultAddPrinter::slotProcessFinished(int iCode, QProcess::ExitStatus exitStatus)
{
    qCInfo(COMMONMOUDLE) << iCode << exitStatus;
    if (exitStatus != QProcess::NormalExit || 0 != iCode) {
        int index;
        m_strErr = m_proc.readAllStandardError();
        index = m_strErr.indexOf("\n") + 1;
        if (index < m_strErr.length())
            m_strErr = m_strErr.mid(index);

        m_iStep = STEP_Failed;
    }

    nextStep();
}

AddPrinterTask::AddPrinterTask(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution, const QString &uri, QObject *parent)
    : QObject(parent)
    , m_installDepends(nullptr)
    , m_installDriver(nullptr)
    , m_bQuit(false)
    , m_iStep(STEP_Start)
    , m_fixHplip(nullptr)
{
    m_printer = printer;
    m_solution = solution;
    m_uri = uri;
    connect(this, &AddPrinterTask::signalStatus, this, &AddPrinterTask::slotWriteLog);
}

AddPrinterTask::~AddPrinterTask()
{
    if (m_fixHplip)
        m_fixHplip->deleteLater();
    if (m_installDriver)
        m_installDriver->deleteLater();
    if (m_installDepends)
        m_installDepends->deleteLater();
}

int AddPrinterTask::isUriAndDriverMatched()
{
    // 不是直连打印机不检查驱动是否匹配, 或uri只有一个无需合并
    if (m_printer.strClass.compare("direct") || m_printer.uriList.size() == 1) {
        return 0;
    }

    // 如果是惠普打印机，匹配驱动和uri，hplip的驱动需要用hp:开头的uri添加
    m_uri.clear();
    QString hpuri, usburi;
    for (auto uri : m_printer.uriList) {
        uri.startsWith("hp:") ? hpuri = uri : usburi = uri;
    }

    QString ppd_name = m_solution[CUPS_PPD_NAME].toString();
    bool ishpdrv = isHplipDrv(ppd_name);

    m_uri = ishpdrv ? hpuri : usburi;

    if (m_uri.isEmpty()) {
        m_strErr = tr("URI and driver do not match.");
        if (ishpdrv) {
            m_strErr += tr("Install hplip first and restart the app to install the driver again.");
            if (!m_fixHplip) {
                m_fixHplip = new FixHplipBackend(this);
                connect(m_fixHplip, &FixHplipBackend::signalStatus, this, &AddPrinterTask::slotFixHplipStatus);
                m_fixHplip->startFixed();
                return 1;
            }
        } else {
            m_strErr += tr("Please select an hplip driver and try again.");
        }
        return -1;
    }

    return 0;
}

int AddPrinterTask::checkUriAndDriver()
{
    QString ppd_name;

    if (m_printer.uriList.isEmpty()) {
        m_strErr = tr("URI can't be empty");

        return -1;
    }

    ppd_name = m_solution[CUPS_PPD_NAME].toString();
    if (m_solution[SD_KEY_from].toInt() == PPDFrom_File && !QFile::exists(ppd_name)) {
        m_strErr = ppd_name + tr(" not found");

        return -2;
    }

    return isUriAndDriverMatched();
}

int AddPrinterTask::doWork()
{
    qCDebug(COMMONMOUDLE) << m_printer.uriList << m_solution[CUPS_PPD_NAME].toString();

    nextStep();

    return 0;
}

void AddPrinterTask::nextStep()
{
    int iRet = 0;

    qCInfo(COMMONMOUDLE) << m_iStep;

    //每一步执行如果返回0则说明执行完，直接执行下一步
    //如果返回大于0则说明有异步的操作在执行，直接返回，等待异步操作执行完调用nextStep
    //如果返回小于0则说明执行失败，将标志设置为失败执行nextStep触发失败的信号
    switch (m_iStep++) {
    case STEP_Start:
        iRet = checkUriAndDriver();
        break;
    case STEP_FillInfo:
        iRet = fillPrinterInfo();
        break;
    case STEP_FixDriver:
        iRet = fixDriverDepends();
        break;
    case STEP_InstallDriver:
        iRet = installDriver();
        break;
    case STEP_AddPrinter:
        iRet = addPrinter();
        break;
    case STEP_Finished:
        emit signalStatus(TStat_Suc);
        return;
    default:
        emit signalStatus(TStat_Fail);
        return;
    }

    if (0 > iRet) {
        m_iStep = STEP_Failed;
    } else if (iRet > 0) {
        return;
    }

    nextStep();
}

QString AddPrinterTask::getErrorMassge()
{
    return m_strErr;
}

TDeviceInfo AddPrinterTask::getPrinterInfo()
{
    return m_printer;
}

QMap<QString, QVariant> AddPrinterTask::getDriverInfo()
{
    return m_solution;
}

int AddPrinterTask::fixDriverDepends()
{
    if (m_solution[SD_KEY_from].toInt() == PPDFrom_EveryWhere)
        return 0;

    QString ppd_name = m_solution[CUPS_PPD_NAME].toString();
    checkPPDName(ppd_name);
    QStringList depends = g_driverManager->getDriverDepends(ppd_name.toUtf8().data());
    if (!depends.isEmpty()) {
        m_installDepends = new InstallInterface(this);
        QList<TPackageInfo> packages;
        foreach (const QString &package, depends) {
            TPackageInfo info;
            info.packageName = package;
            packages.append(info);
        }
        m_installDepends->setPackages(packages);
        connect(m_installDepends, &InstallInterface::signalStatus, this, &AddPrinterTask::slotDependsStatus);
        m_installDepends->startInstallPackages();

        return 1;
    }

    return 0;
}

int AddPrinterTask::installDriver()
{
    if (m_solution[SD_KEY_from].toInt() == PPDFrom_Server) {
        m_installDriver = new InstallDriver(m_solution, this);
        connect(m_installDriver, &InstallDriver::signalStatus, this, &AddPrinterTask::slotInstallStatus);
        m_installDriver->doWork();

        return 1;
    }

    return 0;
}

int AddPrinterTask::fillPrinterInfo()
{
    m_printer.strName = g_addPrinterFactoty->defaultPrinterName(m_printer, m_solution);

    if (m_printer.strLocation.isEmpty()) {
        QString strUri = m_printer.uriList[0];
        QString strHost = getHostFromUri(strUri);

        if (strHost.isEmpty()) {
            if (strUri.startsWith("hp") || strUri.startsWith("usb")) {
                strHost = "Direct-attached Device";
            } else if (strUri.startsWith("file")) {
                strHost = "File";
            }
        }

        m_printer.strLocation = strHost;
    }

    if (m_printer.strInfo.isEmpty()) {
        QString strModel = m_solution[CUPS_PPD_MODEL].toString();

        m_printer.strInfo = strModel.isEmpty() ? m_solution[CUPS_PPD_MAKE_MODEL].toString() : strModel;
    }

    return 0;
}

void AddPrinterTask::stop()
{
    m_bQuit = true;

    if (m_installDriver) {
        disconnect(m_installDriver, &InstallDriver::signalStatus, this, &AddPrinterTask::slotInstallStatus);
        m_installDriver->stop();
    }
}

QStringList GetSystemPrinterNames()
{
    QStringList printerNames;
    map<string, map<string, string>> printers;
    map<string, map<string, string>>::iterator itmap;

    try {
        auto conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            printers = conPtr->getPrinters();

        for (itmap = printers.begin(); itmap != printers.end(); itmap++) {
            printerNames << STQ(itmap->first);
        }
    } catch (const std::exception &ex) {
        qCWarning(COMMONMOUDLE) << "Got execpt: " << QString::fromUtf8(ex.what());
    };

    qCDebug(COMMONMOUDLE) << printerNames;
    return printerNames;
}

void AddPrinterTask::slotDependsStatus(int iStatus)
{
    if (m_bQuit)
        return;

    if (TStat_Fail == iStatus) {
        m_strErr = m_installDepends->getErrorString();
        m_iStep = STEP_Failed;
    }
    nextStep();
}

void AddPrinterTask::slotInstallStatus(int iStatus)
{
    if (m_bQuit)
        return;

    if (TStat_Fail == iStatus) {
        m_strErr = m_installDriver->getErrorString();
        m_iStep = STEP_Failed;
    }
    nextStep();
}

void AddPrinterTask::slotFixHplipStatus(int iStatus)
{
    if (m_bQuit)
        return;

    if (TStat_Fail == iStatus) {
        m_strErr = m_fixHplip->getErrorString();
        m_iStep = STEP_Failed;
    } else if (TStat_Suc == iStatus) {
        QString strUri = m_fixHplip->getMatchHplipUri(m_printer.uriList[0]);

        if (!strUri.isEmpty()) {
            m_uri = strUri;
        }
    }
    nextStep();
}

void AddPrinterTask::slotWriteLog(int status)
{
    if (!isEventSdkInit()) {
        loadEventlib();
    }

    pfWriteEventLog WriteEventLog = getWriteEventLog();
    if (!WriteEventLog) {
        return;
    }

    TDeviceInfo printerInfo = getPrinterInfo();
    QMap<QString, QVariant> driverInfo = getDriverInfo();

    QJsonObject obj;
    QString from = getDriverFrom(driverInfo[SD_KEY_from].toInt());
    QString searchMethod = getSearchType(printerInfo.iType);

    QString ppdMake = driverInfo[CUPS_PPD_MAKE_MODEL].toString();
    ppdMake = ppdMake.contains("(recommended)") ? ppdMake.remove(" (recommended)") : ppdMake;
    obj.insert("driverInfo", QJsonValue(ppdMake));
    obj.insert("printerInfo", QJsonValue(printerInfo.strMakeAndModel));
    obj.insert("serial", QJsonValue(printerInfo.serial));
    obj.insert("printerName", QJsonValue(printerInfo.strName));
    obj.insert("printerUri", QJsonValue(m_uri));

    if (driverInfo[SD_KEY_from].toInt() == PPDFrom_Server) { // 网络方式存在包名和版本号信息
        obj.insert("packageName", QJsonValue(driverInfo[SD_KEY_driver].toString()));
        obj.insert("packageVer", QJsonValue(driverInfo[SD_KEY_debver].toString()));
    }

    QStringList timeListVal = getCurrentTime(FINISH_TIME);
    QString finishTime = timeListVal.join(" ");
    (void)getCurrentTime(RESET_TIME);
    obj.insert("ppdInfo", QJsonValue(driverInfo[CUPS_PPD_NAME].toString()));
    obj.insert("from", QJsonValue(from));
    obj.insert("searchType", QJsonValue(searchMethod));
    obj.insert("finishTime", QJsonValue(finishTime));
    obj.insert("status", QJsonValue(status == TStat_Suc ? "Success" : "Failed"));

    if (status != TStat_Suc) {
        QString reason = getErrorMassge();
        obj.insert("reason", QJsonValue(reason));
    }
    obj.insert("version", getPackageVersion(APPNAME));
    obj.insert("cupsVersion", getPackageVersion("cups"));
    obj.insert("tid", 1000100000); // 事件ID

    QString installInfo = QString(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    WriteEventLog(installInfo.toStdString());
}

AddPrinterFactory *AddPrinterFactory::getInstance()
{
    static AddPrinterFactory *instance = nullptr;
    if (!instance)
        instance = new AddPrinterFactory();

    return instance;
}

AddPrinterTask *AddPrinterFactory::createAddPrinterTask(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution)
{
    QString ppd_name;
    QString device_uri = printer.uriList[0];

    ppd_name = solution[CUPS_PPD_NAME].toString();
    if (ppd_name.isEmpty())
        return nullptr;

    qCInfo(COMMONMOUDLE) << "add printer task:" << solution[SD_KEY_from] << ppd_name;
    /* Canon CAPT local printer must use ccp backend */
    if (isCanonCAPTDrv(ppd_name)) {
        QString info = containsIpAddress(device_uri) ? device_uri : printer.serial;
        return new AddCanonCAPTPrinter(printer, solution, info);
    }

    return new DefaultAddPrinter(printer, solution, device_uri);
}

QString AddPrinterFactory::defaultPrinterName(const TDeviceInfo &printer, const QMap<QString, QVariant> &solution)
{
    QString strDefaultName;
    QString strName = printer.strName;
    QStringList installedPrinters = GetSystemPrinterNames();

    //提前替换一次，防止替换之后变为空字符串
    strName.replace(QRegularExpression("[^\\w-]"), " ");
    QStringList list = strName.split(" ", QString::SkipEmptyParts);
    strName = list.join(" ");
    if (strName.isEmpty()) {
        QString strMM = printer.strMakeAndModel.isEmpty() ? solution.value(CUPS_PPD_MAKE_MODEL).toString() : printer.strMakeAndModel;
        QString strMake = solution.value(CUPS_PPD_MAKE).toString();

        if (strMM.isEmpty() && !strMake.isEmpty()) {
            strMM = strMake + " " + solution.value(CUPS_PPD_MODEL).toString();
        }

        //EveryWhere 不用makeandmodel作为名称，因为会包含中文
        //优先用打印机信息中包含的型号信息，URI中的打印机名字可能是中文
        if (printer.strMakeAndModel.isEmpty() && (strMM.isEmpty() || PPDFrom_EveryWhere == solution[SD_KEY_from].toInt())) {
            strName = getPrinterNameFromUri(printer.uriList[0]);
        } else {
            QString strModel;
            ppdMakeModelSplit(strMM, strMake, strModel);
            strName = strMake + " " + strModel;
        }
    }

    strName.replace(QRegularExpression("[^\\w-]"), " ");
    //去掉多个连续空格的情况
    list = strName.split(" ", QString::SkipEmptyParts);
    strName = list.join(" ");
    if (strName.isEmpty()) {
        strDefaultName = "printer";
    } else {
        strName.replace(" ", "-");
        strDefaultName = strName;
    }

    /*
     * 网络打印机在局域网中存在多台同型号时，普通用户无法通过ip区分打印机，如果用户设置了打印机位置属性
     * 比如办公室等，显示在界面方便用户区分打印机。
    */
    if (printer.uriList.count() > 0) {
        QString protocol = printer.uriList.at(0).left(printer.uriList.at(0).indexOf(":/"));
        if (protocol == "socket" && !printer.strLocation.isEmpty() && !isIpv4Address(printer.strLocation)) {
            QString tmpLocation = printer.strLocation;
            QStringList tmpList = tmpLocation.split(" ", QString::SkipEmptyParts);
            tmpLocation = tmpList.join(" ");
            tmpLocation.replace(QRegularExpression("[/ ?'#\"\\\\]"), "-");
            strDefaultName += "-" + tmpLocation;
        }
    }

    strName = strDefaultName;
    // 保证和已安装的打印机名字不重复
    int i = 1;
    while (installedPrinters.contains(strDefaultName)) {
        strDefaultName = strName + "-" + QString::number(i++);
    }

    // 打印机名字不能超过128个字符
    if (strDefaultName.length() >= 128) {
        strDefaultName = strDefaultName.left(127);
    }
    return strDefaultName;
}
