// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "writedbusdata.h"

#include "../../deepin-pc-manager/src/window/modules/common/common.h"
#include "../../deepin-pc-manager/src/window/modules/common/database/trashcleanuninstallappsql.h"
#include "disk/disk.h"
#include "localcache/securitypkgnamesql.h"
#include "localcache/settingssql.h"

#include <DTrashManager>

#include <QDBusConnection>
#include <QDBusContext>
#include <QDBusMetaType>
#include <QDateTime>
#include <QDebug>
#include <QMetaType>
#include <QSqlError>
#include <QSqlQuery>
#include <QtConcurrent/QtConcurrent>

// 合法调用进程路径列表
const QStringList ValidInvokerExePathList = {"/usr/bin/deepin-pc-manager",
                                             "/usr/bin/deepin-pc-manager-session-daemon",
                                             "/usr/bin/deepin-pc-manager-system-daemon"};

WriteDBusData::WriteDBusData(QObject *parent)
    : QObject(parent)
    , m_daemonAdaptor(nullptr)
    , m_settingsSql(new SettingsSql("WriteDBusData_Settings", this))
    , m_isInitSqlDatabase(false)
{
    m_daemonAdaptor = new DaemonAdaptor(this);
    if (!QDBusConnection::systemBus().registerService(service)
        || !QDBusConnection::systemBus().registerObject(path, this)) {
        exit(0);
    }

    // 初始化localcache.db数据库
    initLocalCache();

    RefreshPackageTable();

    // 扫描冗余文件并删除
    scanFile("/usr/share/deepin-pc-manager/config/");
}

WriteDBusData::~WriteDBusData()
{
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
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    qApp->quit();
}

bool WriteDBusData::SetSysSettingValue(const QString &key, const QDBusVariant &value)
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return false;
    }
    QVariant lastValueOfKey = m_settingsSql->getValue(key);
    QVariant settingValueOfKey = value.variant();
    bool success = m_settingsSql->setValue(key, settingValueOfKey);
    if (lastValueOfKey != settingValueOfKey) {
        Q_EMIT SysSettingValueChanged(key, value);
        qInfo() << "[WriteDBusData] [SetSysSettingValue] key:" << key
                << " value:" << value.variant();
    }

    return success;
}

QDBusVariant WriteDBusData::GetSysSettingValue(const QString &key)
{
    QDBusVariant dbusVariant;
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return dbusVariant;
    }

    QVariant var = m_settingsSql->getValue(key);
    // 未找到值
    if (var.isNull()) {
        qDebug() << Q_FUNC_INFO << "not found value of key:" << key;
        var = 0;
        qWarning() << "[WriteDBusData] [SetSysSettingValue] not found key:" << key;
    }

    qInfo() << "[WriteDBusData] [SetSysSettingValue] var:" << var;
    dbusVariant.setVariant(var);
    return dbusVariant;
}

// 初始化localcache.db数据库
void WriteDBusData::initLocalCache()
{
    // 初始化数据库
    QDir defenderDataDir;
    if (!defenderDataDir.exists(DEFENDER_DATA_DIR_PATH)) {
        defenderDataDir.mkdir(DEFENDER_DATA_DIR_PATH);
    }

    // 在打开之前判断数据库文件是否已存在，若不存在创建并修改其文件属性，存在则进行判断和修改
    QString strDbFileName = QString("%1%2").arg(DEFENDER_DATA_DIR_PATH).arg(LOCAL_CACHE_DB_NAME);
    QFile dbFile(strDbFileName);

    if (dbFile.exists()) {
        if (dbFile.isOpen()) {
            dbFile.close();
        }

        QFile::Permissions permissons = dbFile.permissions();

        if (permissons != (QFile::Permission::WriteUser | QFile::Permission::ReadOwner)) {
            if (!dbFile.setPermissions(QFile::Permission::WriteUser
                                       | QFile::Permission::ReadOwner)) {
                qCritical()
                    << "[WriteDBusData] [initLocalCache] change database permission failed!";
            }
        }
    } else {
        if (!openLocalDb()) {
            ExitApp();
        }

        m_localCacheDb.close();

        // 修改数据库文件权限
        if (!dbFile.setPermissions(QFile::Permission::WriteUser | QFile::Permission::ReadOwner)) {
            qCritical() << "[WriteDBusData] [initLocalCache] modify database permission failed!";
        }
    }

    if (!openLocalDb()) {
        ExitApp();
    }

    // 1.包名及相关联文件数据库表
    m_securityPkgNameSql = new SecurityPkgNameSql(m_localCacheDb);
    connect(this,
            &WriteDBusData::requestRefreshPackageTable,
            m_securityPkgNameSql,
            &SecurityPkgNameSql::refreshPackageLinkTable);
    connect(m_securityPkgNameSql,
            &SecurityPkgNameSql::notifySqlInsertFinish,
            this,
            &WriteDBusData::acceptSqlInsertFinish);

    m_securityPkgNameSqlThread = new QThread;
    m_securityPkgNameSql->moveToThread(m_securityPkgNameSqlThread);
    connect(m_securityPkgNameSqlThread,
            &QThread::started,
            m_securityPkgNameSql,
            &SecurityPkgNameSql::createPackageLinkTable);
}

// 接受数据库插入完成信号
void WriteDBusData::acceptSqlInsertFinish()
{
    if (!m_localCacheDb.isOpen()) {
        qCritical() << "[WriteDBusData] [acceptSqlInsertFinish] pkgName localcachedb open error!";
        return;
    }

    // 查询packageName字段所有数据
    QSqlQuery query(m_localCacheDb);
    QString sCmd;
    sCmd = QString("select * from package_binary_file");
    if (!query.exec(sCmd)) {
        qCritical() << "[WriteDBusData] [acceptSqlInsertFinish] query localcachedb error :"
                    << query.lastError();
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
    if (!isValidInvoker()) {
        return "";
    }
    return m_sPkgNameMap.value(sBinaryFile);
}

// 应用程序安装卸载刷新数据库
void WriteDBusData::RefreshPackageTable()
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    if (!m_isInitSqlDatabase) {
        m_securityPkgNameSqlThread->start();
        m_isInitSqlDatabase = true;
    } else {
        Q_EMIT requestRefreshPackageTable();
    }
}

// 获取磁盘设备路径列表
QStringList WriteDBusData::GetDiskDevicePathList()
{
    return getDiskDevicePathList();
}

// 清理选中的垃圾文件
void WriteDBusData::CleanSelectFile(QStringList filePaths)
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    bool isCleanJournal = false;
    foreach (const auto &file, filePaths) {
        if (file.contains("/var/log/journal")) {
            isCleanJournal = true;
            continue;
        }

        if (file.contains("/.local/share/Trash")) {
            // isCleanTrashBox = true;
            // do not remove trashBox here
            continue;
        }

        QFileInfo fileInfo(file);
        if (fileInfo.isFile() || fileInfo.isSymLink()) {
            // QFile::setPermissions(file, QFile::WriteOwner);
            if (!QFile::remove(fileInfo.absoluteFilePath())) {
                qCritical() << "[WriteDBusData] [CleanSelectFile] remove failed :" << file;
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
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }

    // 先停止sshd服务运行
    QProcess procSSHD;
    procSSHD.start("journalctl", {"--rotate", "--vacuum-size=1M"});
    procSSHD.waitForFinished();
    procSSHD.close();
    procSSHD.deleteLater();
}

// 递归函数-获取/usr/share/deepin-pc-manager/config/目录下所有的lock文件
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
            for (const QFileInfo &i :
                 dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden
                                   | QDir::NoSymLinks)) {
                // 递归扫描目录内容
                scanFile(i.absoluteFilePath());
            }
        }
    }

    CleanSelectFile(sFileList);
}

// 垃圾清理数据库插入记录
void WriteDBusData::InsertUninstalledAppRecord(const QString &appID, const QString &appName)
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    // m_uninstallAppSql->executeCmd();
    TrashCleanUninstallAppSql db;
    if (db.init()) {
        db.executeCmd(TrashCleanUninstallAppSql::UninstalledAppSqlCmdType::INSERT_A_RECORD,
                      {appID, appName});
    }
}

// 垃圾清理数据库删除记录
void WriteDBusData::DeleteUninstalledAppRecord(const QString &appID)
{
    if (!isValidInvoker()) {
        qDebug() << Q_FUNC_INFO << "invalid invoker!";
        return;
    }
    // m_uninstallAppSql->executeCmd(TrashCleanUninstallAppSql::UninstalledAppSqlCmdType::DELETE_A_RECORD,
    // {appID});
    TrashCleanUninstallAppSql db;
    if (db.init()) {
        db.executeCmd(TrashCleanUninstallAppSql::UninstalledAppSqlCmdType::DELETE_A_RECORD,
                      {appID});
    }
}

QVariantList WriteDBusData::GetUnInstalledApps()
{
    TrashCleanUninstallAppSql db;
    QVariantList varList;

    if (db.init()) {
        bool queryFinished = false;

        connect(&db,
                &TrashCleanUninstallAppSql::sendQueryResultSignal,
                this,
                [&](QVariantList rst) {
                    varList = rst;
                    disconnect(&db);
                    queryFinished = true;
                });

        db.executeCmd(TrashCleanUninstallAppSql::GET_ALL_RECORDS);
        QTime timer = QTime::currentTime().addMSecs(3000);

        while (QTime::currentTime() < timer && !queryFinished) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        }

        if (!queryFinished) {
            varList.clear();
            disconnect(&db);
            qCritical() << "query uninstalled app failed!";
        }
    }

    return varList;
}

// 校验调用者
bool WriteDBusData::isValidInvoker()
{
#ifdef QT_DEBUG
    return true;
#else
    // 判断是否位自身调用
    if (!calledFromDBus()) {
        return true;
    }

    bool valid = false;
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    // 判断是否存在执行路径且是否存在于可调用者名单中
    uint pid = conn.interface()->servicePid(msg.service()).value();
    QFileInfo f(QString("/proc/%1/exe").arg(pid));
    QString invokerPath = f.canonicalFilePath();

    // 防止读取的执行路径为空，为空时取cmdline内容
    if (invokerPath.isEmpty()) {
        QFile cmdlineFile(QString("/proc/%1/cmdline").arg(pid));
        if (cmdlineFile.open(QIODevice::OpenModeFlag::ReadOnly)) {
            QString sCmdline = cmdlineFile.readAll();
            invokerPath = sCmdline.split(" ").first();
        }
        cmdlineFile.close();
    }

    if (ValidInvokerExePathList.contains(invokerPath)) {
        valid = true;
    }

    // 如果是非法调用者，则输出错误
    if (!valid) {
        sendErrorReply(QDBusError::ErrorType::Failed,
                       QString("(pid: %1)[%2] is not allowed to transfer antiav inter")
                           .arg(pid)
                           .arg(invokerPath));
    }

    return valid;
#endif
}

bool WriteDBusData::openLocalDb()
{
    bool bSuccess = false;

    // 建立长连接
    if (QSqlDatabase::contains("localcache")) {
        m_localCacheDb = QSqlDatabase::database("localcache");
    } else {
        m_localCacheDb = QSqlDatabase::addDatabase("QSQLITE", "localcache");
    }

    m_localCacheDb.setDatabaseName("/usr/share/deepin-pc-manager/localcache.db");
    m_localCacheDb.setUserName("root");
    m_localCacheDb.setPassword("deepin-pc-manager");

    // 打开数据库
    if (!m_localCacheDb.open()) {
        qCritical() << "localcache.db open error!";
        bSuccess = false;
    } else {
        bSuccess = true;
    }

    return bSuccess;
}
