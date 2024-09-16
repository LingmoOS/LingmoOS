// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "writedbusdata.h"
#include "localcache/securitypkgnamesql.h"
#include "../../deepin-defender/src/window/modules/common/common.h"
#include "localcache/netflowdatasql.h"
#include "networkmanager/netmanagermodel.h"
#include "localcache/securitylogsql.h"

#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>

WriteDBusData::WriteDBusData(QObject *parent)
    : QObject(parent)
    , m_adaptor(new MonitorNetFlowAdaptor(this))
    , m_usbManageMonitor(nullptr)
    , m_usbManageMonitorThread(nullptr)
    , m_securityLogSql(nullptr)
    , m_inGettingSSHStatus(false)
    , m_isInitSqlDatabase(false)
{
    if (!QDBusConnection::systemBus().registerService(service) || !QDBusConnection::systemBus().registerObject(path, this)) {
        qDebug() << "dbus service already registered! " << service;

        exit(0);
    }

    // 初始化localcache.db数据库
    initLocalCache();

    // 扫描冗余文件并删除
    scanFile("/usr/share/deepin-defender/config/");

    // 注册DBus传参类型
    registerDefenderProcInfoMetaType();
    registerDefenderProcInfoListMetaType();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &WriteDBusData::onNotifyHeartbeat);
    timer->start(1000 * HEART_BEAT_TIME_OUT);

    //usb connection monitor
    m_usbManageMonitor = new UsbManageMonitor;

    m_usbManageMonitorThread = new QThread;
    m_usbManageMonitor->moveToThread(m_usbManageMonitorThread);
    connect(m_usbManageMonitorThread, &QThread::started, m_usbManageMonitor, &UsbManageMonitor::run);
    m_usbManageMonitorThread->start();
    connect(m_usbManageMonitor, &UsbManageMonitor::sendConnectionRecord, this, &WriteDBusData::SendUsbConnectionInfo);
    connect(m_usbManageMonitor, &UsbManageMonitor::requestShowBlockAllSysNotify, this, &WriteDBusData::requestShowBlockAllSysNotify);
    connect(m_usbManageMonitor, &UsbManageMonitor::requestShowOnlyWhiteListSysNotify, this, &WriteDBusData::requestShowOnlyWhiteListSysNotify);
    //end - usbManageMonitor

    // network manager model
    m_netManagerModel = new NetManagerModel(this);
    connect(m_netManagerModel, &NetManagerModel::sendProcInfoList, this, &WriteDBusData::SendProcInfoList);
    connect(m_netManagerModel, &NetManagerModel::sendAppFlowList, this, &WriteDBusData::SendAppFlowList);
    connect(m_netManagerModel, &NetManagerModel::sendAllAppFlowList, this, &WriteDBusData::SendAllAppFlowList);
    // 包名数据改动后，刷新应用列表
    connect(m_securityPkgNameSql, &SecurityPkgNameSql::notifySqlInsertFinish, m_netManagerModel, &NetManagerModel::installedAppListChanged);

    // 当ssh状态获取完成
    connect(this, &WriteDBusData::SendSSHStatus, this, &WriteDBusData::recSSHStatus);

    // 获取最新的ssh状态，并通知界面
    AsyncGetRemRegisterStatus();
}

WriteDBusData::~WriteDBusData()
{
    // usb管控线程关闭
    m_usbManageMonitor->quit();
    m_usbManageMonitorThread->quit();
    m_usbManageMonitorThread->wait();
    m_usbManageMonitorThread->deleteLater();
    m_usbManageMonitor->deleteLater();

    // 数据库
    m_securityPkgNameSql->quit();
    m_securityPkgNameSqlThread->quit();
    m_securityPkgNameSqlThread->wait();
    m_securityPkgNameSqlThread->deleteLater();
    m_securityPkgNameSql->deleteLater();
}

void WriteDBusData::StartApp()
{
}

void WriteDBusData::ExitApp()
{
    qApp->quit();
}

void WriteDBusData::onNotifyHeartbeat()
{
    Q_EMIT m_adaptor->NotifyHeartbeat();
}

// 初始化localcache.db数据库
void WriteDBusData::initLocalCache()
{
    //初始化数据库
    QDir defenderDataDir;
    if (!defenderDataDir.exists(DEFENDER_DATA_DIR_PATH)) {
        defenderDataDir.mkdir(DEFENDER_DATA_DIR_PATH);
    }
    // 建立长连接
    if (QSqlDatabase::contains("localcache")) {
        m_localCacheDb = QSqlDatabase::database("localcache");
    } else {
        m_localCacheDb = QSqlDatabase::addDatabase("QSQLITE", "localcache");
    }

    m_localCacheDb.setDatabaseName("/usr/share/deepin-defender/localcache.db");
    m_localCacheDb.setUserName("root");
    m_localCacheDb.setPassword("deepin-defender");

    // 打开数据库
    if (!m_localCacheDb.open()) {
        qDebug() << "localcache.db open error!";
        ExitApp();
    }

    // 1.包名及相关联文件数据库表
    m_securityPkgNameSql = new SecurityPkgNameSql(m_localCacheDb);
    connect(this, &WriteDBusData::requestRefreshPackageTable, m_securityPkgNameSql, &SecurityPkgNameSql::refreshPackageLinkTable);
    connect(m_securityPkgNameSql, &SecurityPkgNameSql::notifySqlInsertFinish, this, &WriteDBusData::acceptSqlInsertFinish);

    m_securityPkgNameSqlThread = new QThread;
    m_securityPkgNameSql->moveToThread(m_securityPkgNameSqlThread);
    connect(m_securityPkgNameSqlThread, &QThread::started, m_securityPkgNameSql, &SecurityPkgNameSql::createPackageLinkTable);

    // 2.安全日志数据库表
    m_securityLogSql = new SecurityLogSql("WriteDBusData", this);
}

// 接受数据库插入完成信号
void WriteDBusData::acceptSqlInsertFinish()
{
    if (!m_localCacheDb.isOpen()) {
        qDebug() << "pkgName localcachedb open error!";
        return;
    }

    // 查询packageName字段所有数据
    QSqlQuery query(m_localCacheDb);
    QString sCmd;
    sCmd = QString("select * from package_binary_file");
    if (!query.exec(sCmd)) {
        qDebug() << "query localcachedb error : " << query.lastError();
    } else {
        // 将已有的所有包名插入到QStringList容器当中
        while (query.next()) {
            m_sPkgNameMap[query.value(0).toString()] = query.value(1).toString();
        }
    }

    Q_EMIT NotifySqlInsertFinish();
}

// 查询相关联文件对应的包名信息
QString WriteDBusData::QueryPackageName(const QString sBinaryFile)
{
    return m_sPkgNameMap.value(sBinaryFile);
}

// 应用程序安装卸载刷新数据库
void WriteDBusData::RefreshPackageTable()
{
    if (!m_isInitSqlDatabase) {
        m_securityPkgNameSqlThread->start();
        m_isInitSqlDatabase = true;
    } else {
        Q_EMIT requestRefreshPackageTable();
    }
}

/*---------------------ssh远程登陆端口---------------------*/
// 设置/获取ssh远程登陆端口状态
void WriteDBusData::setRemRegisterStatus(bool bStatus)
{
    if (bStatus) {
        // 先开启ssh服务
        QProcess *procSSH = new QProcess();
        procSSH->start(QString("systemctl enable ssh.service"));
        procSSH->waitForFinished();
        procSSH->deleteLater();

        // 然后运行sshd服务
        QProcess *procSSHD = new QProcess();
        procSSHD->start(QString("systemctl start sshd.service"));
        procSSHD->waitForFinished();
        procSSHD->deleteLater();
    } else {
        // 先停止sshd服务运行
        QProcess *procSSHD = new QProcess();
        procSSHD->start(QString("systemctl stop sshd.service"));
        procSSHD->waitForFinished();
        procSSHD->deleteLater();

        // 然后禁止ssh服务自启
        QProcess *procSSH = new QProcess();
        procSSH->start(QString("systemctl disable ssh.service"));
        procSSH->waitForFinished();
        procSSH->deleteLater();
    }

    // ssh远程登陆端口状态改变
    Q_EMIT SSHStatusChanged(bStatus);
    // 发送ssh远程登陆端口状态
    Q_EMIT SendSSHStatus(bStatus);
}

bool WriteDBusData::getRemRegisterStatus()
{
    QProcess *processSSH = new QProcess();
    processSSH->start("systemctl status sshd.service");
    processSSH->waitForStarted();
    processSSH->waitForFinished();
    QString sProcessResultPath = QString::fromLocal8Bit(processSSH->readAllStandardOutput()).trimmed();
    processSSH->deleteLater();

    bool retOn = sProcessResultPath.contains(REM_SSH_STATUS);
    Q_EMIT SSHStatusChanged(retOn);
    // 发送ssh远程登陆端口状态
    Q_EMIT SendSSHStatus(retOn);
    return retOn;
}

// 异步设置ssh远程登陆端口状态
void WriteDBusData::AsyncSetRemRegisterStatus(bool bStatus)
{
    QtConcurrent::run(this, &WriteDBusData::setRemRegisterStatus, bStatus);
}

// 异步获取ssh远程登陆端口状态
void WriteDBusData::AsyncGetRemRegisterStatus()
{
    // 若正在获取ssh状态, 则返回
    if (m_inGettingSSHStatus) {
        return;
    }

    // 设置正在获取ssh状态
    m_inGettingSSHStatus = true;
    QtConcurrent::run(this, &WriteDBusData::getRemRegisterStatus);
}

// 当接收到ssh状态
void WriteDBusData::recSSHStatus(bool status)
{
    Q_UNUSED(status);
    m_inGettingSSHStatus = false;
}

// 开启/关闭进程流量监控功能
void WriteDBusData::EnableNetFlowMonitor(bool enable)
{
    m_netManagerModel->enableNetFlowMonitor(enable);
}

// 获取磁盘设备路径列表
QStringList WriteDBusData::GetDiskDevicePathList()
{
    return getDiskDevicePathList();
}

// 清理选中的垃圾文件
void WriteDBusData::CleanSelectFile(QStringList filePaths)
{
    bool isCleanJournal = false;
    foreach (const auto &file, filePaths) {
        if (file.contains("/var/log/journal")) {
            isCleanJournal = true;
            continue;
        }

        if (file.contains("/.local/share/Trash")) {
            // do not remove trashBox here
            continue;
        }

        QFileInfo fileInfo(file);
        if (fileInfo.isFile() || fileInfo.isSymLink()) {
            if (!QFile::remove(fileInfo.absoluteFilePath())) {
                qDebug() << "remove failed : " << file;
            }
        }
    }

    if (isCleanJournal) {
        CleanJournal();
    }
}

// 清理journal目录下的日志文件
void WriteDBusData::CleanJournal()
{
    // 先停止sshd服务运行
    QProcess *procSSHD = new QProcess(this);
    procSSHD->start(QString("journalctl --vacuum-size=1M"));
    procSSHD->waitForFinished();
    procSSHD->deleteLater();
}

// 添加安全日志
void WriteDBusData::AddSecurityLog(int nType, QString sInfo)
{
    m_securityLogSql->addSecurityLog(nType, sInfo);
}

// 删除过滤条件下的安全日志
bool WriteDBusData::DeleteSecurityLog(int nLastDate, int nType)
{
    return m_securityLogSql->deleteSecurityLog(nLastDate, nType);
}

// 递归函数-获取/usr/share/deepin-defender/config/目录下所有的lock文件
void WriteDBusData::scanFile(const QString &path)
{
    QStringList sFileList;
    QFileInfo info(path);
    if (info.exists()) {
        if (info.isFile()) {
            QString sPath = info.absoluteFilePath();
            if (sPath.contains("lock")) {
                sFileList.append(sPath);
            }
        } else if (info.isDir()) {
            QDir dir(path);
            for (const QFileInfo &i : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden | QDir::NoSymLinks)) {
                // 递归扫描目录内容
                scanFile(i.absoluteFilePath());
            }
        }
    }

    CleanSelectFile(sFileList);
}

void WriteDBusData::AsyncGetAppFlowList(const QString &sPkgName, const int timeRangeType)
{
    Q_EMIT m_netManagerModel->requestQueryAppFlowList(sPkgName, timeRangeType);
}

void WriteDBusData::AsyncGetAllAppFlowList(const int timeRangeType)
{
    Q_EMIT m_netManagerModel->requestQueryAllAppFlowList(timeRangeType);
}
