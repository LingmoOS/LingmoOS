// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logbackend.h"
#include "logallexportthread.h"
#include "logfileparser.h"
#include "logexportthread.h"
#include "logsettings.h"
#include "utils.h"
#include "dbusmanager.h"
#include "dbusproxy/dldbushandler.h"
#include "logapplicationhelper.h"
#include "DebugTimeManager.h"
#include "eventlogutils.h"
#include "logsegementexportthread.h"
#include "parsethread/parsethreadbase.h"

#include <sys/utsname.h>
#include <malloc.h>

#include <DSysInfo>

#include <QDateTime>
#include <QStandardPaths>
#include <QThreadPool>
#include <QLoggingCategory>
#include <QCoreApplication>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logBackend, "org.deepin.log.viewer.backend")
#else
Q_LOGGING_CATEGORY(logBackend, "org.deepin.log.viewer.backend", QtInfoMsg)
#endif

// 获取窗管崩溃时，其日志最后100行
#define KWIN_LASTLINE_NUM 100
// 窗管二进制可执行文件所在路径
const QString KWAYLAND_EXE_PATH = "/usr/bin/kwin_wayland";
const QString XWAYLAND_EXE_PATH = "/usr/bin/Xwayland";

LogBackend *LogBackend::m_staticbackend = nullptr;

LogBackend *LogBackend::instance(QObject *parent)
{
    if (parent != nullptr && m_staticbackend == nullptr) {
        m_staticbackend = new LogBackend(parent);
    }

    return m_staticbackend;
}

LogBackend::~LogBackend()
{

}

LogBackend::LogBackend(QObject *parent) : QObject(parent)
{
    getLogTypes();

    m_cmdWorkDir = QDir::currentPath();
    Utils::m_mapAuditType2EventType = LogSettings::instance()->loadAuditMap();

    initConnections();
}

void LogBackend::initConnections()
{
    connect(&m_logFileParser, &LogFileParser::parseFinished, this, &LogBackend::slot_parseFinished,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::logData, this, &LogBackend::slot_logData,
            Qt::QueuedConnection);

    connect(&m_logFileParser, &LogFileParser::dpkgFinished, this, &LogBackend::slot_dpkgFinished,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::dpkgData, this, &LogBackend::slot_dpkgData,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::xlogFinished, this, &LogBackend::slot_XorgFinished,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::xlogData, this, &LogBackend::slot_xorgData,
            Qt::QueuedConnection);

    connect(&m_logFileParser, &LogFileParser::bootFinished, this, &LogBackend::slot_bootFinished,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::bootData, this, &LogBackend::slot_bootData,
            Qt::QueuedConnection);

    connect(&m_logFileParser, &LogFileParser::kernFinished, this, &LogBackend::slot_kernFinished,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::kernData, this, &LogBackend::slot_kernData,
            Qt::QueuedConnection);

    connect(&m_logFileParser, &LogFileParser::journalFinished, this, &LogBackend::slot_journalFinished,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::journalData, this, &LogBackend::slot_journalData,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::journaBootlData, this, &LogBackend::slot_journalBootData,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::appFinished, this,
            &LogBackend::slot_applicationFinished);
    connect(&m_logFileParser, &LogFileParser::appData, this,
            &LogBackend::slot_applicationData);

    connect(&m_logFileParser, &LogFileParser::kwinFinished, this, &LogBackend::slot_kwinFinished,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::kwinData, this, &LogBackend::slot_kwinData,
            Qt::QueuedConnection);

    connect(&m_logFileParser, &LogFileParser::normalData, this, &LogBackend::slot_normalData,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::normalFinished, this, &LogBackend::slot_normalFinished,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::journalBootFinished, this, &LogBackend::slot_journalBootFinished,
            Qt::QueuedConnection);

    connect(&m_logFileParser, &LogFileParser::proccessError, this, &LogBackend::slot_logLoadFailed,
            Qt::QueuedConnection);
    connect(&m_logFileParser, SIGNAL(dnfFinished(QList<LOG_MSG_DNF>)), this, SLOT(slot_dnfFinished(QList<LOG_MSG_DNF>)));
    connect(&m_logFileParser, &LogFileParser::dmesgFinished, this, &LogBackend::slot_dmesgFinished,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::OOCData, this, &LogBackend::slot_OOCData,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::OOCFinished, this, &LogBackend::slot_OOCFinished,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::auditData, this, &LogBackend::slot_auditData,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::auditFinished, this, &LogBackend::slot_auditFinished,
            Qt::QueuedConnection);

    connect(&m_logFileParser, &LogFileParser::coredumpData, this, &LogBackend::slot_coredumpData,
            Qt::QueuedConnection);
    connect(&m_logFileParser, &LogFileParser::coredumpFinished, this, &LogBackend::slot_coredumpFinished,
            Qt::QueuedConnection);
}

void LogBackend::setCmdWorkDir(const QString &dirPath)
{
    QDir dir(dirPath);
    if (dir.exists())
        m_cmdWorkDir = dirPath;
}

int LogBackend::exportAllLogs(const QString &outDir)
{
    if(!getOutDirPath(outDir))
        return -1;

    PERF_PRINT_BEGIN("POINT-05", "export all logs");
    qCInfo(logBackend) << "exporting all logs begin.";

    // 时间
    QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");

    // 主机名
    utsname _utsname;
    uname(&_utsname);
    QString hostname = QString(_utsname.nodename);
    QString fileFullPath = m_outPath + "/" + QString("%1_%2_all_logs.zip").arg(dateTime).arg(hostname);

    // 添加文件后缀
    if (!fileFullPath.endsWith(".zip")) {
        fileFullPath += ".zip";
    }

    LogAllExportThread *thread = new LogAllExportThread(m_logTypes, fileFullPath);
    thread->setAutoDelete(true);
    connect(thread, &LogAllExportThread::exportFinsh, this, [ = ](bool ret) {
        if (ret) {
            qCInfo(logBackend) << "exporting all logs done.";
            PERF_PRINT_END("POINT-05", "cost");
            qApp->quit();
        } else {
            qCWarning(logBackend) << "exporting all logs stoped.";
            // 导出失败，若为用户指定的新目录，应清除
            if (m_newDir) {
                QDir odir(m_outPath);
                odir.removeRecursively();
            }
            PERF_PRINT_END("POINT-05", "cost");
            qApp->exit(-1);
        }
    });
    QThreadPool::globalInstance()->start(thread);

    return 0;
}

int LogBackend::exportTypeLogs(const QString &outDir, const QString &type)
{
    // 输出目录有效性验证
    if(!getOutDirPath(outDir))
        return -1;

    // 日志种类有效性验证
    QString error;
    m_flag = type2Flag(type, error);
    if (NONE == m_flag) {
        qCWarning(logBackend) << error;
        return -1;
    }

    QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    QString categoryOutPath = QString("%1/%2-%3/").arg(m_outPath).arg(type).arg(dateTime);

    qCInfo(logBackend) << "exporting ... type:" << type;
    bool bSuccess = true;
    switch (m_flag) {
    case JOURNAL: {
        resetCategoryOutputPath(categoryOutPath);

        DLDBusHandler::instance()->exportLog(categoryOutPath, "journalctl_system", false);
    }
    break;
    case Dmesg: {
        resetCategoryOutputPath(categoryOutPath);

        DLDBusHandler::instance()->exportLog(categoryOutPath, "dmesg", false);
    }
    break;
    case KERN: {
        QStringList logPaths = DLDBusHandler::instance()->getFileInfo("kern", false);
        if (logPaths.size() > 0) {
            resetCategoryOutputPath(categoryOutPath);

            for (auto &file: logPaths)
                DLDBusHandler::instance()->exportLog(categoryOutPath, file, true);
        } else if (logPaths.size() == 0) {
            qCWarning(logBackend) << "/var/log has not kern.log";
            bSuccess = false;
        }
    }
    break;
    case BOOT_KLU: {
        resetCategoryOutputPath(categoryOutPath);

        DLDBusHandler::instance()->exportLog(categoryOutPath, "journalctl_boot", false);
    }
    break;
    case BOOT: {
        QStringList logPaths = DLDBusHandler::instance()->getFileInfo("boot", false);
        if (!logPaths.isEmpty()) {
            resetCategoryOutputPath(categoryOutPath);

            for (auto &file: logPaths)
                DLDBusHandler::instance()->exportLog(categoryOutPath, file, true);
        } else {
            qCWarning(logBackend) << "/var/log has not boot.log";
            bSuccess = false;
        }
    }
    break;
    case DPKG: {
        QStringList logPaths = DLDBusHandler::instance()->getFileInfo("dpkg", false);
        if (!logPaths.isEmpty()) {
            resetCategoryOutputPath(categoryOutPath);

            for (auto &file: logPaths)
                DLDBusHandler::instance()->exportLog(categoryOutPath, file, true);
        } else {
            qCWarning(logBackend) << "/var/log has not dpkg.log";
            bSuccess = false;
        }
    }
    break;
    case Dnf: {
        QStringList logPaths = DLDBusHandler::instance()->getFileInfo("dnf", false);
        if (!logPaths.isEmpty()) {
            resetCategoryOutputPath(categoryOutPath);

            for (auto &file: logPaths)
                DLDBusHandler::instance()->exportLog(categoryOutPath, file, true);
        } else {
            qCWarning(logBackend) << "/var/log has not dnf.log";
            bSuccess = false;
        }
    }
    break;
    case Kwin: {
        resetCategoryOutputPath(categoryOutPath);

        DLDBusHandler::instance()->exportLog(categoryOutPath, KWIN_TREE_DATA, true);
    }
    break;
    case XORG: {
        QStringList logPaths = DLDBusHandler::instance()->getFileInfo("Xorg", false);
        if (!logPaths.isEmpty()) {
            resetCategoryOutputPath(categoryOutPath);

            for (auto &file: logPaths)
                DLDBusHandler::instance()->exportLog(categoryOutPath, file, true);
        } else {
            qCWarning(logBackend) << "/var/log has not Xorg.log";
            bSuccess = false;
        }
    }
    break;
    case APP: {
        categoryOutPath = QString("%1/%2").arg(m_outPath).arg("apps");
        resetCategoryOutputPath(categoryOutPath);

        bSuccess = false;
        AppLogConfigList appConfigs = LogApplicationHelper::instance()->getAppLogConfigs();
        for (auto appConfig : appConfigs) {
            QString appName = appConfig.name;
            // 隐藏的应用不导出日志
            if (appName.isEmpty() || !appConfig.visible)
                continue;

            // 逐个导出应用日志
            if (exportAppLogs(categoryOutPath, appName) != -1)
                bSuccess = true;
        }
    }
    break;
    case COREDUMP: {
        QStringList logPaths = DLDBusHandler::instance()->getFileInfo("coredump", false);
        if (!logPaths.isEmpty()) {
            resetCategoryOutputPath(categoryOutPath);

            for (auto &file: logPaths)
                DLDBusHandler::instance()->exportLog(categoryOutPath, file, true);
        } else {
            qCWarning(logBackend) << "/var/log has no coredump logs";
            bSuccess = false;
        }
    }
    break;
    case Normal: {
        QFile file("/var/log/wtmp");
        if (!file.exists()) {
            qCWarning(logBackend) << "/var/log has no boot shutdown event log";
            bSuccess = false;
        }

        resetCategoryOutputPath(categoryOutPath);

        DLDBusHandler::instance()->exportLog(categoryOutPath, "last", false);
    }
    break;
    case OtherLog: {
        categoryOutPath = QString("%1/%2/").arg(m_outPath).arg("others");
        resetCategoryOutputPath(categoryOutPath);

        auto otherLogListPair = LogApplicationHelper::instance()->getOtherLogList();
        for (auto &it2 : otherLogListPair) {
            QStringList logPaths = DLDBusHandler::instance(nullptr)->getOtherFileInfo(it2.at(1));
            logPaths.removeDuplicates();
            if (logPaths.size() > 1) {
                QString tmpSubCategoryOutPath = QString("%1/%2/").arg(categoryOutPath).arg(it2.at(0));
                Utils::mkMutiDir(tmpSubCategoryOutPath);
                for (auto &file : logPaths) {
                    DLDBusHandler::instance()->exportLog(tmpSubCategoryOutPath, file, true);
                }
            }
            else if (logPaths.size() == 1)
                 DLDBusHandler::instance()->exportLog(categoryOutPath, logPaths[0], true);
        }
    }
    break;
    case CustomLog: {
        auto customLogListPair = LogApplicationHelper::instance()->getCustomLogList();
        QStringList logPaths;
        for (auto &it2 : customLogListPair) {
            logPaths.append(it2.at(1));
        }

        if (logPaths.size() > 0) {
            categoryOutPath = QString("%1/%2/").arg(m_outPath).arg("customized");
            resetCategoryOutputPath(categoryOutPath);

            for (auto &file : logPaths) {
                DLDBusHandler::instance()->exportLog(categoryOutPath, file, true);
            }
        } else {
            qCWarning(logBackend) << "no custom logs";
            bSuccess = false;
        }
    }
    break;
    case Audit: {
        QStringList logPaths = DLDBusHandler::instance()->getFileInfo("audit", false);
        if (!logPaths.isEmpty()) {
            resetCategoryOutputPath(categoryOutPath);

            for (auto &file: logPaths)
                DLDBusHandler::instance()->exportLog(categoryOutPath, file, true);
        } else {
            qCWarning(logBackend) << "/var/log has no audit logs";
            bSuccess = false;
        }
    }
    break;
    default:
        break;
    }

    Utils::resetToNormalAuth(categoryOutPath);

    if (bSuccess)
        qCInfo(logBackend) << QString("export success.");
    else
        qCInfo(logBackend) << QString("export failed.");

    return bSuccess ? 0 : -1;
}

bool LogBackend::LogBackend::exportTypeLogsByCondition(const QString &outDir, const QString &type, const QString &period, const QString &condition, const QString &keyword)
{
    if (!getOutDirPath(outDir))
        return false;

    // 日志种类有效性验证
    QString error;
    m_flag = type2Flag(type, error);
    if (NONE == m_flag) {
        qCWarning(logBackend) << error;
        return false;
    }

    qCInfo(logBackend) << "exporting ... type:" << type;

    m_currentSearchStr = keyword;

    m_sessionType = Export;

    // 解析数据
    if (!parseData(m_flag, period, condition)) {
        qCWarning(logBackend) << QString("parse data failed.");
        return false;
    }

    m_sessionType = Export;
    return true;
}

int LogBackend::exportAppLogs(const QString &outDir, const QString &appName)
{
    if(!getOutDirPath(outDir))
        return -1;

    if (appName.isEmpty())
        return -1;

    // 先查找是否有该应用相关日志配置
    if (!LogApplicationHelper::instance()->isAppLogConfigExist(appName)) {
        qCWarning(logBackend) << QString("unknown app:%1 is invalid.").arg(appName);
        return -1;
    }

    qCInfo(logBackend) << QString("exporting %1 logs...").arg(appName);

    QString categoryOutPath = QString("%1/%2").arg(m_outPath).arg(appName);

    bool bSuccess = false;
    AppLogConfig appLogConfig = LogApplicationHelper::instance()->appLogConfig(appName);
    for (int i = 0; i < appLogConfig.subModules.size(); i++) {
        SubModuleConfig& submodule = appLogConfig.subModules[i];
        QString subCategoryOutPath = QString("/%1/%2/").arg(categoryOutPath).arg(submodule.name);
        if (appLogConfig.subModules.size() == 1 &&  submodule.name == appLogConfig.name)
            subCategoryOutPath = categoryOutPath;
        if (submodule.logType == "file") {
            QStringList logPaths = DLDBusHandler::instance(nullptr)->getFileInfo(submodule.logPath);
            logPaths.removeDuplicates();
            if (logPaths.size() > 0) {
                resetCategoryOutputPath(subCategoryOutPath);

                for (auto &file: logPaths)
                    DLDBusHandler::instance()->exportLog(subCategoryOutPath, file, true);
                bSuccess = true;
            } else {
                qCWarning(logBackend) << QString("app:%1 submodule:%2, logPath:%3 not found log files.").arg(appName).arg(submodule.name).arg(submodule.logPath);
            }
        } else if (submodule.logType == "journal") {
            if (submodule.filter.endsWith("*"))
                qCWarning(logBackend) << QString("app:%1 submodule:%2, Export journal logs with wildcard not supported.").arg(appName).arg(submodule.name);
            else {
                QJsonObject obj = submodule.toJson();
                resetCategoryOutputPath(subCategoryOutPath);
                DLDBusHandler::instance()->exportLog(subCategoryOutPath, QJsonDocument(obj).toJson(QJsonDocument::Compact), false);
                bSuccess = true;
            }
        }
    }

    Utils::resetToNormalAuth(categoryOutPath);

    if (bSuccess)
        qCInfo(logBackend) << QString("export %1 logs success.").arg(appName) << "\n";
    else
        qCInfo(logBackend) << QString("export %1 logs failed.").arg(appName) << "\n";

    return bSuccess ? 0 : -1;
}

bool LogBackend::exportAppLogsByCondition(const QString &outDir, const QString &appName, const QString &period, const QString &level, const QString &submodule, const QString &keyword)
{
    if(!getOutDirPath(outDir))
        return false;

    if (appName.isEmpty())
        return false;

    // 周期类型有效性验证
    BUTTONID periodId = ALL;
    if (!period.isEmpty()) {
        periodId = period2Enum(period);
        if (INVALID == periodId) {
            qCWarning(logBackend) << "invalid 'period' parameter: " << period << "\nUSEAGE: all(export all), today(export today), 3d(export past 3 days), 1w(export past week), 1m(export past month), 3m(export past 3 months)";
            return false;
        }
    }

    // 级别有效性判断
    if (!level.isEmpty() && level2Id(level) == -2) {
        qCWarning(logBackend) << "invalid 'level' parameter: " << level << "\nUSEAGE: all(all), 0(emerg), 1(alert), 2(crit), 3(error), 4(warning), 5(notice), 6(info), 7(debug)";
        return false;
    }

    qCInfo(logBackend) << "appName:" << appName << "period:" << period << "level:" << level << "submodule" << submodule << "keyword:" << keyword;

    TIME_RANGE timeRange = getTimeRange(periodId);

    // 先查找是否有该应用相关日志配置
    if (!LogApplicationHelper::instance()->isAppLogConfigExist(appName)) {
        qCWarning(logBackend) << QString("unknown app:%1 is invalid.").arg(appName);
        return false;
    }

    m_flag = APP;
    m_curApp = appName;
    m_sessionType = Export;
    m_currentSearchStr = keyword;

    m_appFilter.clear();
    m_appFilter.app = appName;
    m_appFilter.submodule = submodule;
    m_appFilter.lvlFilter = level2Id(level);
    m_appFilter.timeFilterBegin = timeRange.begin;
    m_appFilter.timeFilterEnd = timeRange.end;
    m_appFilter.searchstr = m_currentSearchStr;

    m_appCurrentIndex = m_logFileParser.parseByApp(m_appFilter);

    return true;
}

QStringList LogBackend::getLogTypes()
{
    Dtk::Core::DSysInfo::UosEdition edition = Dtk::Core::DSysInfo::uosEditionType();
    //等于服务器行业版或欧拉版(centos)
    bool isCentos = Dtk::Core::DSysInfo::UosEuler == edition || Dtk::Core::DSysInfo::UosEnterpriseC == edition || Dtk::Core::DSysInfo::UosMilitaryS == edition;
    if (QFile::exists("/var/log/journal") || isCentos) {
        m_logTypes.push_back(JOUR_TREE_DATA);
    }

    if (isCentos) {
        m_logTypes.push_back(DMESG_TREE_DATA);
    } else {
        if (QFile::exists("/var/log/kern.log")) {
            m_logTypes.push_back(KERN_TREE_DATA);
        }
    }
    if (Utils::isWayland()) {
        m_logTypes.push_back(BOOT_KLU_TREE_DATA);
    } else {
        m_logTypes.push_back(BOOT_TREE_DATA);
    }
    if (isCentos) {
        m_logTypes.push_back(DNF_TREE_DATA);
    } else {
        if (QFile::exists("/var/log/dpkg.log")) {
            m_logTypes.push_back(DPKG_TREE_DATA);
        }
    }
    //wayland环境才有kwin日志
    if (Utils::isWayland()) {
        m_logTypes.push_back(KWIN_TREE_DATA);
    } else {
        m_logTypes.push_back(XORG_TREE_DATA);
    }
    auto *appHelper = LogApplicationHelper::instance();
    QMap<QString, QString> appMap = appHelper->getMap();
    if (!appMap.isEmpty()) {
        m_logTypes.push_back(APP_TREE_DATA);
    }

    m_logTypes.push_back(COREDUMP_TREE_DATA);

    // add by Airy
    if (QFile::exists("/var/log/wtmp")) {
        m_logTypes.push_back(LAST_TREE_DATA);
    }

    //other
    m_logTypes.push_back(OTHER_TREE_DATA);

    //custom
    if (LogApplicationHelper::instance()->getCustomLogList().size()) {
        m_logTypes.push_back(CUSTOM_TREE_DATA);
    }

    // 审计日志文件存在，才加载和显示审计日志模块（审计日志文件需要root权限访问，因此在service服务中判断审计日志文件是否存在）
    if (DLDBusHandler::instance(qApp)->isFileExist(AUDIT_TREE_DATA)) {
        m_logTypes.push_back(AUDIT_TREE_DATA);
    }

    return m_logTypes;
}

void LogBackend::slot_parseFinished(int index, LOG_FLAG type, int status)
{
    if (m_flag != type || index != m_type2ThreadIndex[type])
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit parseFinished(type, status);

        if (status != ParseThreadBase::Normal)
            return;
    } else if (Export == m_sessionType) {
        // 导出当前解析到的数据
        executeCLIExport(m_exportFilePath);

        // 异常情况，不再进行分段导出
        if (status != ParseThreadBase::Normal) {
            // 还原任务状态
            if (View == m_lastSessionType) {
                m_sessionType = View;
                m_bSegementExporting = false;
                Utils::checkAndDeleteDir(m_exportFilePath);
                // 通知前端，处理进度条隐藏问题
                emit parseFinished(type, status);
            }
            return;
        }

        // 若有分段数据，开启分段导出
        segementExport();
    }
}

void LogBackend::slot_logData(int index, const QList<QString> &list, LOG_FLAG type)
{
    if (m_flag != type || index != m_type2ThreadIndex[type])
        return;

    m_type2LogDataOrigin[type].append(list);
    QList<QString> filterData = filterLog(m_currentSearchStr, list);
    m_type2LogData[type].append(filterData);

    if (View == m_sessionType) {
        // 只中转新增数据到界面
        emit logData(filterData, type);
    }
}

void LogBackend::slot_dpkgFinished(int index)
{
    if (m_flag != DPKG || index != m_dpkgCurrentIndex)
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit dpkgFinished();
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_dpkgData(int index, QList<LOG_MSG_DPKG> list)
{
    if (m_flag != DPKG || index != m_dpkgCurrentIndex)
        return;

    dListOrigin.append(list);
    dList.append(filterDpkg(m_currentSearchStr, list));

    if (View == m_sessionType) {
        emit dpkgData(dList);
    }
}

void LogBackend::slot_XorgFinished(int index)
{
    if (m_flag != XORG || index != m_xorgCurrentIndex)
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit xlogFinished();
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_xorgData(int index, QList<LOG_MSG_XORG> list)
{
    if (m_flag != XORG || index != m_xorgCurrentIndex)
        return;

    xListOrigin.append(list);
    xList.append(filterXorg(m_currentSearchStr, list));

    if (View == m_sessionType) {
        emit xlogData(xList);
    }
}

void LogBackend::slot_bootFinished(int index)
{
    if (m_flag != BOOT || index != m_bootCurrentIndex)
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit bootFinished();
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_bootData(int index, QList<LOG_MSG_BOOT> list)
{
    if (m_flag != BOOT || index != m_bootCurrentIndex)
        return;

    bList.append(list);
    currentBootList.append(filterBoot(m_bootFilter, list));

    if (View == m_sessionType) {
        emit bootData(currentBootList);
    }
}

void LogBackend::slot_kernFinished(int index)
{
    if (m_flag != KERN || index != m_kernCurrentIndex)
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit kernFinished();
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_kernData(int index, QList<LOG_MSG_JOURNAL> list)
{
    if (m_flag != KERN || index != m_kernCurrentIndex)
        return;

    kListOrigin.append(list);
    kList.append(filterKern(m_currentSearchStr, list));

    if (View == m_sessionType) {
        emit kernData(kList);
    }
}

void LogBackend::slot_kwinFinished(int index)
{
    if (m_flag != Kwin || index != m_kwinCurrentIndex)
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit kwinFinished();
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_kwinData(int index, QList<LOG_MSG_KWIN> list)
{
    if (m_flag != Kwin || index != m_kwinCurrentIndex)
        return;
    m_kwinList.append(list);
    m_currentKwinList.append(filterKwin(m_currentSearchStr, list));

    if (View == m_sessionType) {
        emit kwinData(m_currentKwinList);
    }
}

void LogBackend::slot_dnfFinished(const QList<LOG_MSG_DNF> &list)
{
    if (m_flag != Dnf)
        return;
    dnfList = filterDnf(m_currentSearchStr, list);
    dnfListOrigin = list;

    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit dnfFinished(dnfListOrigin);
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_dmesgFinished(const QList<LOG_MSG_DMESG> &list)
{
    if (m_flag != Dmesg)
        return;

    dmesgList = filterDmesg(m_currentSearchStr,list);
    dmesgListOrigin = list;

    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit dmesgFinished(dmesgListOrigin);
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_journalFinished(int index)
{
    if (m_flag != JOURNAL || index != m_journalCurrentIndex)
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit journalFinished();
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_journalBootFinished(int index)
{
    if (m_flag != BOOT_KLU || index != m_journalBootCurrentIndex)
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit journalBootFinished();
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_journalBootData(int index, QList<LOG_MSG_JOURNAL> list)
{
    if (m_flag != BOOT_KLU || index != m_journalBootCurrentIndex)
        return;

    jBootListOrigin.append(list);
    jBootList.append(filterJournalBoot(m_currentSearchStr, list));

    if (View == m_sessionType) {
        emit journaBootlData(jBootList);
    }
}

void LogBackend::slot_journalData(int index, QList<LOG_MSG_JOURNAL> list)
{
    //判断最近一次获取数据线程的标记量,和信号曹发来的sender的标记量作对比,如果相同才可以刷新,因为会出现上次的获取线程就算停下信号也发出来了
    if (m_flag != JOURNAL || index != m_journalCurrentIndex)
        return;

    jListOrigin.append(list);
    jList.append(filterJournal(m_currentSearchStr, list));

    if (View == m_sessionType) {
        emit journalData(jList);
    }
}

void LogBackend::slot_applicationFinished(int index)
{
    if (m_flag != APP || index != m_appCurrentIndex)
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit appFinished();
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_applicationData(int index, QList<LOG_MSG_APPLICATOIN> list)
{
    if (m_flag != APP || index != m_appCurrentIndex)
        return;

    appListOrigin.append(list);
    appList.append(filterApp(m_appFilter, list));

    if (View == m_sessionType) {
        emit appData(appList);
    }
}

void LogBackend::slot_normalFinished(int index)
{
    if (m_flag != Normal || index != m_normalCurrentIndex)
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit normalFinished();
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_normalData(int index, QList<LOG_MSG_NORMAL> list)
{
    if (m_flag != Normal || index != m_normalCurrentIndex)
        return;
    norList.append(list);
    nortempList.append(filterNomal(m_normalFilter, list));

    if (View == m_sessionType) {
        emit normalData(nortempList);
    }
}

void LogBackend::slot_OOCFinished(int index, int error)
{
    if ((m_flag != OtherLog && m_flag != CustomLog) || index != m_OOCCurrentIndex)
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType)
        emit OOCFinished(error);
}

void LogBackend::slot_OOCData(int index, const QString &data)
{
    if ((m_flag != OtherLog && m_flag != CustomLog) || index != m_OOCCurrentIndex)
        return;

    if (View == m_sessionType)
        emit OOCData(data);
}

void LogBackend::slot_auditFinished(int index, bool bShowTip)
{
    Q_UNUSED(bShowTip);

    if (m_flag != Audit || index != m_auditCurrentIndex)
        return;
    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit auditFinished();
    } else if (Export == m_sessionType) {
        executeCLIExport();
    }
}

void LogBackend::slot_auditData(int index, QList<LOG_MSG_AUDIT> list)
{
    if (m_flag != Audit || index != m_auditCurrentIndex)
        return;

    aListOrigin.append(list);
    aList.append(filterAudit(m_auditFilter, list));

    if (View == m_sessionType)
        emit auditData(aList);
}

void LogBackend::slot_coredumpFinished(int index)
{
    if (m_flag != COREDUMP || index != m_coredumpCurrentIndex)
        return;

    m_isDataLoadComplete = true;

    if (View == m_sessionType) {
        emit coredumpFinished();
    } else if (Export == m_sessionType) {
        executeCLIExport();
    } else if (Report == m_sessionType) {
        int nCount = m_currentCoredumpList.size();
        QDateTime lastTime = LogApplicationHelper::instance()->getLastReportTime();
        QDateTime curTime = QDateTime::currentDateTime();
        if (nCount == 0) {
            qCWarning(logBackend) << QString("Report coredump info failed, timeRange: '%1 ---- %2', no matching data.")
                                     .arg(lastTime.toString("yyyy-MM-dd hh:mm:ss"))
                                     .arg(curTime.toString("yyyy-MM-dd hh:mm:ss"));
            // 此处退出码不能为-1，否则systemctl --failed服务会将其判为失败的systemd服务
            qApp->exit(0);
        } else {
            // 统计所有崩溃重复次数
            QJsonObject repeatObj;
            QList<LOG_REPEAT_COREDUMP_INFO> repeatInfos = Utils::countRepeatCoredumps();
            for (auto i : repeatInfos) {
                repeatObj.insert(i.exePath, i.times);
            }

            // 更新高频崩溃应用exe路径名单（更新范围为上次上报到当前时间新产生的崩溃信息），并同步到配置文件中
            QList<LOG_REPEAT_COREDUMP_INFO> repeatInfoInTimeRange = Utils::countRepeatCoredumps(lastTime.toMSecsSinceEpoch(), curTime.toMSecsSinceEpoch());
            Utils::updateRepeatCoredumpExePaths(repeatInfoInTimeRange);
            QStringList repeatCoredumpExePaths = Utils::getRepeatCoredumpExePaths();

            // 获取最大上报条数
            const int nMaxCoredumpReport = LogApplicationHelper::instance()->getMaxReportCoredump();

            // 数据清洗，去重
            QList<LOG_MSG_COREDUMP> afterCleanData;
            for (const auto &data : m_currentCoredumpList) {
                if (repeatCoredumpExePaths.indexOf(data.exe) != -1) {
                    auto it = std::find_if(afterCleanData.begin(), afterCleanData.end(),[&](const LOG_MSG_COREDUMP &item) {
                        return item.exe == data.exe;
                    });

                    // 清洗时，只保留最近的一条重复数据
                    if (it != afterCleanData.end())
                        continue;
                }

                afterCleanData.push_back(data);

                // 限制要上报的崩溃数据条数，默认最多上报50条。若nMaxCoredumpReport <= 0，则取消最大上报条数限制
                if (afterCleanData.size() > nMaxCoredumpReport && nMaxCoredumpReport > 0)
                    break;
            }

            // 仅清洗后的数据，才解析崩溃详情数据
            parseCoredumpDetailInfo(afterCleanData);

            // 崩溃数据转json数据
            QJsonArray objList;
            QDateTime latestCoredumpTime;
            for (auto &data : afterCleanData) {
                QDateTime coredumpTime = QDateTime::fromString(data.dateTime, "yyyy-MM-dd hh:mm:ss");
                if (coredumpTime > latestCoredumpTime)
                    latestCoredumpTime = coredumpTime;
                objList.append(data.toJson());
            }

            // 以最近的崩溃时间的下一秒作为下次上报的筛选起始时间
            latestCoredumpTime = latestCoredumpTime.addSecs(1);

            // 先初始化埋点接口，延迟2秒后调用埋点接口，以便能正常写入埋点数据
            Eventlogutils::GetInstance();

            QTimer::singleShot(2000, this, [=]{
                // 埋点记录崩溃数据
                QJsonObject objCoredumpEvent{
                    {"tid", Eventlogutils::ReportCoredump},
                    {"version", QCoreApplication::applicationVersion()},
                    {"event",  "log"},
                    {"target", "coredump"},
                    {"coredumpListCount", DLDBusHandler::instance()->executeCmd("coredumpctl-list-count").toInt()},
                    {"repeatCoredumps", repeatObj},
                    {"message", objList}
                };

                Eventlogutils::GetInstance()->writeLogs(objCoredumpEvent);
                LogApplicationHelper::instance()->saveLastRerportTime(latestCoredumpTime);
                qCInfo(logBackend) << QString("Successfully reported %1 crash messages in total.").arg(afterCleanData.size());
                qApp->exit(0);
            });
        }
    }
}

void LogBackend::slot_coredumpData(int index, QList<LOG_MSG_COREDUMP> list)
{
    if (m_flag != COREDUMP || index != m_coredumpCurrentIndex)
        return;

    m_coredumpList.append(list);
    QList<LOG_MSG_COREDUMP> filterList = filterCoredump(m_currentSearchStr, list);
    m_currentCoredumpList.append(filterList);

    if (View == m_sessionType)
        emit coredumpData(m_currentCoredumpList, !filterList.isEmpty());
}

void LogBackend::slot_logLoadFailed(const QString &iError)
{
    qCWarning(logBackend) << "parse data failed. error: " << iError;

    if (View == m_sessionType)
        emit proccessError(iError);
    else if (Export == m_sessionType || Report == m_sessionType)
        qApp->exit(-1);
}

void LogBackend::onExportProgress(int nCur, int nTotal)
{
    if (View == m_sessionType || View == m_lastSessionType) {
        bool bExportThreadRunning = false;

        // 获取导出线程对象
        LogExportThread *exportThread = qobject_cast<LogExportThread *>(sender());
        LogSegementExportThread* segementExportThread = qobject_cast<LogSegementExportThread *>(sender());
        if (exportThread)
            bExportThreadRunning = exportThread->isProcessing();
        else if (segementExportThread) {
            bExportThreadRunning = segementExportThread->isProcessing();
        }

        //如果导出线程不再运行则不处理此信号
        if (!bExportThreadRunning) {
            return;
        }

        emit sigProgress(nCur, nTotal);
    }
}

void LogBackend::onExportResult(bool isSuccess)
{
    if (View == m_sessionType) {
        emit sigResult(isSuccess);
    } else if (Export == m_sessionType || Report == m_sessionType) {
        if (Export == m_sessionType)
            Utils::resetToNormalAuth(m_exportFilePath);

        PERF_PRINT_END("POINT-04", "");
        if (isSuccess) {
            qCInfo(logBackend) << "export success.";
            qApp->quit();
        } else {
            qCWarning(logBackend) << "export failed.";
            qApp->exit(-1);
        }
    }
}

void LogBackend::onExportFakeCloseDlg()
{
    if (View == m_sessionType) {
        emit sigProcessFull();
    }
}

QList<LOG_MSG_BOOT> LogBackend::filterBoot(BOOT_FILTERS ibootFilter, const QList<LOG_MSG_BOOT> &iList)
{
    QList<LOG_MSG_BOOT> rsList;
    bool isStatusFilterEmpty = ibootFilter.statusFilter.isEmpty();
    if (isStatusFilterEmpty && ibootFilter.searchstr.isEmpty()) {
        return iList;
    } else {
        for (int i = 0; i < iList.size(); i++) {
            LOG_MSG_BOOT msg = iList.at(i);
            QString _statusStr = msg.status;
            if ((_statusStr.compare(ibootFilter.statusFilter, Qt::CaseInsensitive) != 0) && !isStatusFilterEmpty)
                continue;
            if ((msg.status.contains(ibootFilter.searchstr, Qt::CaseInsensitive)) || (msg.msg.contains(ibootFilter.searchstr, Qt::CaseInsensitive))) {
                rsList.append(iList[i]);
            }
        }
    }
    return rsList;
}

QList<LOG_MSG_NORMAL> LogBackend::filterNomal(NORMAL_FILTERS inormalFilter, const QList<LOG_MSG_NORMAL> &iList)
{
    QList<LOG_MSG_NORMAL> rsList;
    if (inormalFilter.searchstr.isEmpty() && inormalFilter.eventTypeFilter < 0) {
        return iList;
    }
    int tcbx = inormalFilter.eventTypeFilter;
    if (0 == tcbx) {
        for (auto i = 0; i < iList.size(); i++) {
            LOG_MSG_NORMAL msg = iList.at(i);
            if (msg.eventType.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.userName.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.dateTime.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.msg.contains(inormalFilter.searchstr, Qt::CaseInsensitive)) {
                rsList.append(msg);
            }
        }
    } else if (1 == tcbx) {
        for (auto i = 0; i < iList.size(); i++) {
            LOG_MSG_NORMAL msg = iList.at(i);
            if (msg.eventType.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.userName.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.dateTime.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.msg.contains(inormalFilter.searchstr, Qt::CaseInsensitive)) {
                if (msg.eventType.compare("Boot", Qt::CaseInsensitive) != 0 && msg.eventType.compare("shutdown", Qt::CaseInsensitive) != 0 && msg.eventType.compare("runlevel", Qt::CaseInsensitive) != 0)
                    rsList.append(msg);
            }
        }
    } else if (2 == tcbx) {
        for (auto i = 0; i < iList.size(); i++) {
            LOG_MSG_NORMAL msg = iList.at(i);
            if (msg.eventType.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.userName.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.dateTime.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.msg.contains(inormalFilter.searchstr, Qt::CaseInsensitive)) {
                if (iList[i].eventType.compare("Boot", Qt::CaseInsensitive) == 0)
                    rsList.append(iList[i]);
            }
        }
    } else if (3 == tcbx) {
        for (auto i = 0; i < iList.size(); i++) {
            LOG_MSG_NORMAL msg = iList.at(i);
            if (msg.eventType.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.userName.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.dateTime.contains(inormalFilter.searchstr, Qt::CaseInsensitive) || msg.msg.contains(inormalFilter.searchstr, Qt::CaseInsensitive)) {
                if (iList[i].eventType.compare("shutdown", Qt::CaseInsensitive) == 0)
                    rsList.append(iList[i]);
            }
        }
    }
    return rsList;
}

QList<LOG_MSG_DPKG> LogBackend::filterDpkg(const QString &iSearchStr, const QList<LOG_MSG_DPKG> &iList)
{
    QList<LOG_MSG_DPKG> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }

    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_DPKG msg = iList.at(i);
        if (msg.dateTime.contains(iSearchStr, Qt::CaseInsensitive) || msg.msg.contains(iSearchStr, Qt::CaseInsensitive)) {
            rsList.append(msg);
        }
    }
    return rsList;
}

QList<LOG_MSG_JOURNAL> LogBackend::filterKern(const QString &iSearchStr, const QList<LOG_MSG_JOURNAL> &iList)
{
    QList<LOG_MSG_JOURNAL> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }
    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_JOURNAL msg = iList.at(i);
        if (msg.dateTime.contains(iSearchStr, Qt::CaseInsensitive) || msg.hostName.contains(iSearchStr, Qt::CaseInsensitive) || msg.daemonName.contains(iSearchStr, Qt::CaseInsensitive) || msg.msg.contains(iSearchStr, Qt::CaseInsensitive)) {
            rsList.append(msg);
        }
    }
    return rsList;
}

QList<LOG_MSG_XORG> LogBackend::filterXorg(const QString &iSearchStr, const QList<LOG_MSG_XORG> &iList)
{
    QList<LOG_MSG_XORG> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }
    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_XORG msg = iList.at(i);
        if (msg.offset.contains(iSearchStr, Qt::CaseInsensitive) || msg.msg.contains(iSearchStr, Qt::CaseInsensitive))
            rsList.append(msg);
    }
    return rsList;
}

QList<LOG_MSG_KWIN> LogBackend::filterKwin(const QString &iSearchStr, const QList<LOG_MSG_KWIN> &iList)
{
    QList<LOG_MSG_KWIN> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }
    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_KWIN msg = iList.at(i);
        if (msg.msg.contains(iSearchStr, Qt::CaseInsensitive))
            rsList.append(msg);
    }
    return rsList;
}

QList<LOG_MSG_APPLICATOIN> LogBackend::filterApp(const QString &iSearchStr, const QList<LOG_MSG_APPLICATOIN> &iList)
{
    QList<LOG_MSG_APPLICATOIN> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }
    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_APPLICATOIN msg = iList.at(i);
        if (msg.dateTime.contains(iSearchStr, Qt::CaseInsensitive) || msg.level.contains(iSearchStr, Qt::CaseInsensitive) || msg.src.contains(iSearchStr, Qt::CaseInsensitive) || msg.msg.contains(iSearchStr, Qt::CaseInsensitive))
            rsList.append(msg);
    }
    return rsList;
}

QList<LOG_MSG_APPLICATOIN> LogBackend::filterApp(APP_FILTERS appFilter, const QList<LOG_MSG_APPLICATOIN> &iList)
{
    QList<LOG_MSG_APPLICATOIN> rsList;
    if (appFilter.searchstr.isEmpty() && appFilter.submodule.isEmpty()) {
        return iList;
    }

    if (appFilter.submodule.isEmpty()) {
        for (int i = 0; i < iList.size(); i++) {
            LOG_MSG_APPLICATOIN msg = iList.at(i);
            if (msg.dateTime.contains(appFilter.searchstr, Qt::CaseInsensitive)
                    || msg.level.contains(appFilter.searchstr, Qt::CaseInsensitive)
                    || msg.src.contains(appFilter.searchstr, Qt::CaseInsensitive)
                    || msg.msg.contains(appFilter.searchstr, Qt::CaseInsensitive)
                    || msg.subModule.contains(appFilter.searchstr, Qt::CaseInsensitive)) {
                rsList.append(msg);
            }
        }
    } else {
        for (int i = 0; i < iList.size(); i++) {
            LOG_MSG_APPLICATOIN msg = iList.at(i);
            if (msg.dateTime.contains(appFilter.searchstr, Qt::CaseInsensitive)
                    || msg.level.contains(appFilter.searchstr, Qt::CaseInsensitive)
                    || msg.src.contains(appFilter.searchstr, Qt::CaseInsensitive)
                    || msg.msg.contains(appFilter.searchstr, Qt::CaseInsensitive)
                    || msg.subModule.contains(appFilter.searchstr, Qt::CaseInsensitive)) {
                if (msg.subModule.compare(appFilter.submodule, Qt::CaseInsensitive) == 0) {
                    rsList.append(msg);
                }
            }
        }
    }
    return rsList;
}

QList<LOG_MSG_DNF> LogBackend::filterDnf(const QString &iSearchStr, const QList<LOG_MSG_DNF> &iList)
{
    QList<LOG_MSG_DNF> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }
    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_DNF msg = iList.at(i);
        if (msg.dateTime.contains(iSearchStr, Qt::CaseInsensitive)
                || msg.msg.contains(iSearchStr, Qt::CaseInsensitive)
                || msg.level.contains(iSearchStr, Qt::CaseInsensitive))
            rsList.append(msg);
    }
    return rsList;
}

QList<LOG_MSG_DMESG> LogBackend::filterDmesg(const QString &iSearchStr, const QList<LOG_MSG_DMESG> &iList)
{
    QList<LOG_MSG_DMESG> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }

    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_DMESG msg = iList.at(i);
        if (msg.dateTime.contains(iSearchStr, Qt::CaseInsensitive) || msg.msg.contains(iSearchStr, Qt::CaseInsensitive))
            rsList.append(msg);
    }

    return rsList;
}

QList<LOG_MSG_JOURNAL> LogBackend::filterJournal(const QString &iSearchStr, const QList<LOG_MSG_JOURNAL> &iList)
{
    QList<LOG_MSG_JOURNAL> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }
    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_JOURNAL msg = iList.at(i);
        if (msg.dateTime.contains(iSearchStr, Qt::CaseInsensitive) || msg.hostName.contains(iSearchStr, Qt::CaseInsensitive) || msg.daemonName.contains(iSearchStr, Qt::CaseInsensitive) || msg.daemonId.contains(iSearchStr, Qt::CaseInsensitive) || msg.level.contains(iSearchStr, Qt::CaseInsensitive) || msg.msg.contains(iSearchStr, Qt::CaseInsensitive))
            rsList.append(msg);
    }
    return rsList;
}

QList<LOG_MSG_JOURNAL> LogBackend::filterJournalBoot(const QString &iSearchStr, const QList<LOG_MSG_JOURNAL> &iList)
{
    QList<LOG_MSG_JOURNAL> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }
    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_JOURNAL msg = iList.at(i);
        if (msg.dateTime.contains(iSearchStr, Qt::CaseInsensitive) || msg.hostName.contains(iSearchStr, Qt::CaseInsensitive) || msg.daemonName.contains(iSearchStr, Qt::CaseInsensitive) || msg.daemonId.contains(iSearchStr, Qt::CaseInsensitive) || msg.level.contains(iSearchStr, Qt::CaseInsensitive) || msg.msg.contains(iSearchStr, Qt::CaseInsensitive))
            rsList.append(msg);
    }
    return rsList;
}

QList<LOG_FILE_OTHERORCUSTOM> LogBackend::filterOOC(const QString &iSearchStr, const QList<LOG_FILE_OTHERORCUSTOM> &iList)
{
    QList<LOG_FILE_OTHERORCUSTOM> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }
    for (int i = 0; i < iList.size(); i++) {
        LOG_FILE_OTHERORCUSTOM msg = iList.at(i);
        if (msg.name.contains(iSearchStr, Qt::CaseInsensitive) || msg.path.contains(iSearchStr, Qt::CaseInsensitive))
            rsList.append(msg);
    }
    return rsList;
}

QList<LOG_MSG_AUDIT> LogBackend::filterAudit(AUDIT_FILTERS auditFilter, const QList<LOG_MSG_AUDIT> &iList)
{
    QList<LOG_MSG_AUDIT> rsList;
    if (auditFilter.searchstr.isEmpty() && auditFilter.auditTypeFilter < -1) {
        return iList;
    }
    int nAuditType = auditFilter.auditTypeFilter - 1;
    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_AUDIT msg = iList.at(i);
        if (msg.contains(auditFilter.searchstr) && (nAuditType == -1 || msg.filterAuditType(nAuditType))) {
            rsList.append(msg);
        }
    }

    return rsList;
}
QList<LOG_MSG_COREDUMP> LogBackend::filterCoredump(const QString &iSearchStr, const QList<LOG_MSG_COREDUMP> &iList)
{
    QList<LOG_MSG_COREDUMP> rsList;
    if (iSearchStr.isEmpty()) {
        return iList;
    }
    for (int i = 0; i < iList.size(); i++) {
        LOG_MSG_COREDUMP msg = iList.at(i);
        if (msg.sig.contains(iSearchStr, Qt::CaseInsensitive)
                || msg.dateTime.contains(iSearchStr, Qt::CaseInsensitive)
                || msg.coreFile.contains(iSearchStr, Qt::CaseInsensitive)
                || msg.userName.contains(iSearchStr, Qt::CaseInsensitive)
                || msg.exe.contains(iSearchStr, Qt::CaseInsensitive)) {
            rsList.append(msg);
        }
    }

    return rsList;
}

bool LogBackend::parseData(const LOG_FLAG &flag, const QString &period, const QString &condition)
{
    // 周期类型有效性验证
    BUTTONID periodId = ALL;
    if (!period.isEmpty()) {
        periodId = period2Enum(period);
        if (INVALID == periodId) {
            qCWarning(logBackend) << "invalid 'period' parameter: " << period << "\nUSEAGE: all(export all), today(export today), 3d(export past 3 days), 1w(export past week), 1m(export past month), 3m(export past 3 months)";
            return false;
        }
    }

    // 常规级别有效性判断
    int lId = -1;
    if (flag == JOURNAL || flag == Dmesg || flag == BOOT_KLU || flag == APP) {
        if (!condition.isEmpty()) {
            lId = level2Id(condition);
            if (-2 == lId) {
                qCWarning(logBackend) << "invalid 'level' parameter: " << condition << "\nUSEAGE: 0(emerg), 1(alert), 2(crit), 3(error), 4(warning), 5(notice), 6(info), 7(debug)";
                return false;
            }
        }

    }

    // dnf级别有效性判断
    DNFPRIORITY dnfLevel = DNFLVALL;
    if (flag == Dnf) {
        if (!condition.isEmpty()) {
            dnfLevel = dnfLevel2Id(condition);
            if (DNFINVALID == dnfLevel) {
                qCWarning(logBackend) << "invalid 'level' parameter: " << condition << "\nUSEAGE: 0(supercrit), 1(crit), 2(error), 3(warning), 4(info), 5(debug), 6(trace)";
                return false;
            }
        }
    }

    // boot status有效性判断
    QString statusFilter = "";
    if (flag == BOOT) {
        if (!condition.isEmpty()) {
            if (condition == "0")
                statusFilter = "";
            else if (condition == "ok" || condition == "1")
                statusFilter = "OK";
            else if (condition == "failed" || condition == "2")
                statusFilter = "Failed";
            else {
                qCWarning(logBackend) << "invalid 'status' parameter: " << condition << "\nUSEAGE: 0(export all), 1(export ok), 2(export failed)";
                return false;
            }
        }
    }

    // boot-shutdown-event event类型有效性判断
    int normalEventType = -1;
    if (flag == Normal) {
        if (!condition.isEmpty()) {
            normalEventType = normal2eventType(condition);
            if (-1 == normalEventType) {
                qCWarning(logBackend) << "invalid 'event' parameter: " << condition << "\nUSEAGE: 0(export all), 1(export login), 2(export boot), 3(shutdown)";
                return false;
            }
        }
    }

    // audit event有效性判断
    int auditType = 0;
    if (flag == Audit) {
        if (!condition.isEmpty()) {
            auditType = audit2eventType(condition);
            if (-1 == auditType) {
                qCWarning(logBackend) << "invalid 'event' parameter: " << condition << "\nUSEAGE: 0(all), 1(ident auth), 2(discretionary access Contro), 3(mandatory access control), 4(remote), 5(doc audit), 6(other)";
                return false;
            }
        }
    }

    qCInfo(logBackend) << "parsing ..." << "period:" << period << "condition:" << condition << "keyword:" << m_currentSearchStr;

    TIME_RANGE timeRange = getTimeRange(periodId);

    // 设置筛选条件，解析数据
    switch (flag) {
    case JOURNAL: {
        QStringList arg;
        if (lId != LVALL) {
            QString prio = QString("PRIORITY=%1").arg(lId);
            arg.append(prio);
        } else {
            arg.append("all");
        }

        if (timeRange.begin != -1 && timeRange.end != -1) {
            arg << QString::number(timeRange.begin * 1000) << QString::number(timeRange.end *1000);
        }

        m_journalCurrentIndex = m_logFileParser.parseByJournal(arg);
    }
    break;
    case Dmesg: {
        DMESG_FILTERS dmesgfilter;
        dmesgfilter.levelFilter = static_cast<PRIORITY>(lId);
        dmesgfilter.timeFilter = timeRange.begin;
        if (periodId == ALL)
            dmesgfilter.timeFilter = 0;

        m_logFileParser.parseByDmesg(dmesgfilter);
    }
    break;
    case KERN: {
        LOG_FILTER_BASE filter;
        filter.type = flag;
        filter.filePath = "kern";
        filter.timeFilterBegin = timeRange.begin;
        filter.timeFilterEnd = timeRange.end;
        filter.segementIndex = 0;
        m_type2Filter[flag] = filter;

        m_type2ThreadIndex[flag] = m_logFileParser.parse(filter);
    }
    break;
    case BOOT_KLU: {
        QStringList arg;
        if (lId != LVALL) {
            QString prio = QString("PRIORITY=%1").arg(lId);
            arg.append(prio);
        } else {
            arg.append("all");
        }

        m_journalBootCurrentIndex = m_logFileParser.parseByJournalBoot(arg);
    }
    break;
    case BOOT: {
        m_bootFilter.searchstr = m_currentSearchStr;
        m_bootFilter.statusFilter = statusFilter;

        m_bootCurrentIndex = m_logFileParser.parseByBoot();
    }
    break;
    case DPKG: {
        DKPG_FILTERS dpkgFilter;
        dpkgFilter.timeFilterBegin = timeRange.begin;
        dpkgFilter.timeFilterEnd = timeRange.end;
        m_dpkgCurrentIndex = m_logFileParser.parseByDpkg(dpkgFilter);
    }
    break;
    case Dnf: {
        DNF_FILTERS dnffilter;
        dnffilter.levelfilter = dnfLevel;
        dnffilter.timeFilter = timeRange.begin;
        if (periodId == ALL)
            dnffilter.timeFilter = 0;

        m_logFileParser.parseByDnf(dnffilter);
    }
    break;
    case Kwin: {
        LOG_FILTER_BASE filter;
        filter.type = flag;
        filter.segementIndex = 0;
        m_type2Filter[flag] = filter;

        m_type2ThreadIndex[flag] = m_logFileParser.parse(filter);
    }
    break;
    case XORG: {
        XORG_FILTERS xorgFilter;
        m_xorgCurrentIndex = m_logFileParser.parseByXlog(xorgFilter);
    }
    break;
    case APP: {
        // TODO 待实现
    }
    break;
    case COREDUMP: {
        COREDUMP_FILTERS coreFilter;
        coreFilter.timeFilterBegin = timeRange.begin;
        coreFilter.timeFilterEnd = timeRange.end;
        m_coredumpCurrentIndex = m_logFileParser.parseByCoredump(coreFilter);
    }
    break;
    case Normal: {
        m_normalFilter.searchstr = m_currentSearchStr;
        m_normalFilter.timeFilterBegin = timeRange.begin;
        m_normalFilter.timeFilterEnd = timeRange.end;
        m_normalFilter.eventTypeFilter = normalEventType;
        m_normalCurrentIndex = m_logFileParser.parseByNormal(m_normalFilter);
    }
    break;
    case Audit: {
        m_auditFilter.searchstr = m_currentSearchStr;
        m_auditFilter.timeFilterBegin = timeRange.begin;
        m_auditFilter.timeFilterEnd = timeRange.end;
        m_auditFilter.auditTypeFilter = auditType;
        m_auditCurrentIndex = m_logFileParser.parseByAudit(m_auditFilter);
    }
    break;
    default:
        break;
    }

    return true;
}

void LogBackend::executeCLIExport(const QString &originFilePath)
{
    if (!m_isDataLoadComplete)
        return;

    QString filePath = originFilePath;
    if (originFilePath.isEmpty()) {
        QString outPath = m_outPath;
        switch (m_flag) {
        case JOURNAL: {
            filePath = outPath + "/system.txt";
        }
            break;
        case Dmesg: {
            filePath = outPath + "/dmesg.txt";
        }
            break;
        case KERN: {
            filePath = outPath + "/kernel.txt";
        }
            break;
        case BOOT_KLU: {
            filePath = outPath + "/boot_klu.txt";
        }
            break;
        case BOOT: {
            filePath = outPath + "/boot.txt";
        }
            break;
        case DPKG: {
            filePath = outPath + "/dpkg.txt";
        }
            break;
        case Dnf: {
            filePath = outPath + "/dnf.txt";
        }
            break;
        case Kwin: {
            filePath = outPath + "/kwin.txt";
        }
            break;
        case XORG: {
            filePath = outPath + "/xorg.txt";
        }
            break;
        case APP: {
            filePath = outPath + QString("/%1.txt").arg(m_appFilter.app);
        }
            break;
        case COREDUMP: {
            filePath = outPath + "/coredump.zip";
        }
            break;
        case Normal: {
            filePath = outPath + "/boot-shutdown-event.txt";
        }
            break;
        case Audit: {
            filePath = outPath + "/audit.txt";
        }
            break;
        default:
            break;
        }
    }

    exportLogData(filePath);
}

void LogBackend::resetCategoryOutputPath(const QString &path)
{
    // 先清空原有路径中的kernel日志文件
    QDir dir(path);
    dir.removeRecursively();

    // 创建目录
    Utils::mkMutiDir(path);
}

QString LogBackend::getOutDirPath() const
{
    return m_outPath;
}

bool LogBackend::getOutDirPath(const QString &path)
{
    if (path == m_outPath)
        return true;

    QString tmpPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (!path.isEmpty())
        tmpPath = QDir::isRelativePath(path) ? (m_cmdWorkDir + "/" + path) : path;

    // 若指定目录不存在，先创建目录
    QDir dir(tmpPath);
    if (!dir.exists()) {
         dir.mkpath(dir.absolutePath());
         m_newDir = dir.exists();
         if (m_newDir)
             tmpPath = dir.absolutePath();
    } else {
        tmpPath = dir.absolutePath();
    }

    if (dir.exists()) {
        QFileInfo fi(tmpPath);
        if (!fi.isWritable()) {
            qCWarning(logBackend) << QString("outPath: %1 is not writable.").arg(tmpPath);
            return false;
        }

        // 导出路径白名单检查
        QStringList availablePaths =  DLDBusHandler::instance()->whiteListOutPaths();
        bool bAvailable = false;
        for (auto path : availablePaths) {
            if (tmpPath.startsWith(path)) {
                bAvailable = true;
                break;
            }
        }
        if (!bAvailable) {
            qCWarning(logBackend) << qApp->translate("ExportMessage", "The export directory is not available. Please choose another directory for the export operation.");
            qCWarning(logBackend) << "outPath: " << tmpPath;
            return false;
        }

        m_outPath = tmpPath;
        qCInfo(logBackend) << "outPath:" << m_outPath;
        return true;
    } else {
        m_outPath = "";
        qCWarning(logBackend) << QString("outpath:%1 is not exist.").arg(path);
    }

    return false;
}

LOG_FLAG LogBackend::type2Flag(const QString &type, QString& error)
{
    Dtk::Core::DSysInfo::UosEdition edition = Dtk::Core::DSysInfo::uosEditionType();
    bool isCentos = Dtk::Core::DSysInfo::UosEuler == edition || Dtk::Core::DSysInfo::UosEnterpriseC == edition || Dtk::Core::DSysInfo::UosMilitaryS == edition;

    LOG_FLAG flag = NONE;
    if (type == TYPE_SYSTEM)
        flag = JOURNAL;
    else if (type == TYPE_KERNEL) {
        if (isCentos)
            flag = Dmesg;
        else
            flag = KERN;
    } else if (type == TYPE_BOOT) {
        if (Utils::isWayland())
            flag = BOOT_KLU;
        else
            flag = BOOT;
    } else if (type == TYPE_DPKG) {
        if (!isCentos)
            flag = DPKG;
        else
            error = "Server industry edition has no dpkg.log";
    } else if (type == TYPE_DNF) {
        if (isCentos)
            flag = Dnf;
        else
            error = "Only server industry edition has dnf.log";
    } else if (type == TYPE_KWIN) {
        if (Utils::isWayland())
            flag = Kwin;
        else
            error = "Only wayland platform has kwin.log";
    } else if (type == TYPE_XORG) {
        if (!Utils::isWayland())
            flag = XORG;
        else
            error = "Wayland platform has no Xorg.log";
    } else if (type == TYPE_APP) {
        flag = APP;
    } else if (type == TYPE_COREDUMP) {
        flag = COREDUMP;
    } else if (type == TYPE_BSE) {
        flag = Normal;
    } else if (type == TYPE_OTHER) {
        flag = OtherLog;
    } else if (type == TYPE_CUSTOM) {
        flag = CustomLog;
    } else if (type == TYPE_AUDIT) {
        flag = Audit;
    } else {
        flag = NONE;
        error = QString("Unknown type: %1.").arg(type) + "USEAGE: system(journal log), kernel(kernel log), boot(boot log), dpkg(dpkg log), dnf(dnf log), kwin(Kwin log), xorg(Xorg log), app(deepin app log), coredump(coredump log)、boot-shutdown-event(boot shutdown event log)、other(other log)、custom(custom log)、audit(audit log)";
    }

    return flag;
}

bool LogBackend::reportCoredumpInfo()
{
    qCDebug(logBackend) << "ready report coredump.";

    m_flag = COREDUMP;
    m_currentSearchStr = "";
    m_sessionType = Report;

    // 增量上报，每次上报，仅上报新增的崩溃信息，根据时间范围筛选出目标数据
    COREDUMP_FILTERS coreFilter;
    QDateTime lastTime = LogApplicationHelper::instance()->getLastReportTime();
    QDateTime curTime = QDateTime::currentDateTime();

    // 异常的上次上报时间纠错
    if (lastTime > curTime) {
        qCWarning(logBackend) << QString("last report time:[%1] is invalid, reset to %2").arg(lastTime.toString("yyyy-MM-dd hh:mm:ss")).arg(curTime.toString("yyyy-MM-dd hh:mm:ss"));
        lastTime = curTime;
        LogApplicationHelper::instance()->saveLastRerportTime(curTime);
    }

    if (!lastTime.isValid()) {
        TIME_RANGE timeRange = getTimeRange(ALL);
        coreFilter.timeFilterBegin = timeRange.begin;
        coreFilter.timeFilterEnd = timeRange.end;
    } else {
        coreFilter.timeFilterBegin = lastTime.toMSecsSinceEpoch();
        coreFilter.timeFilterEnd = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    m_coredumpCurrentIndex = m_logFileParser.parseByCoredump(coreFilter, true);

    return true;
}

void LogBackend::setFlag(const LOG_FLAG &flag)
{
    m_flag = flag;
}

/**
 * @brief DisplayContent::clearAllFilter 清空当前所有的筛选条件成员变量(只限在本类中筛选的条件)
 */
void LogBackend::clearAllFilter()
{
    m_type2Filter.clear();

    m_bootFilter = {"", ""};
    m_currentSearchStr.clear();
    m_bootFilter.searchstr = "";
    m_normalFilter.clear();
    m_appFilter.clear();
    m_auditFilter.clear();
}

/**
 * @brief DisplayContent::clearAllDatalist 清空所有获取的数据list
 */
void LogBackend::clearAllDatalist()
{
    m_type2LogData.clear();
    m_type2LogDataOrigin.clear();

    jList.clear();
    jListOrigin.clear();
    dList.clear();
    dListOrigin.clear();
    xList.clear();
    xListOrigin.clear();
    bList.clear();
    currentBootList.clear();
    kList.clear();
    kListOrigin.clear();
    appList.clear();
    appListOrigin.clear();
    norList.clear();
    nortempList.clear();
    m_currentKwinList.clear();
    m_kwinList.clear();
    jBootList.clear();
    jBootListOrigin.clear();
    dnfList.clear();
    dnfListOrigin.clear();
    oList.clear();
    oListOrigin.clear();
    cList.clear();
    cListOrigin.clear();
    aList.clear();
    aListOrigin.clear();
    m_coredumpList.clear();
    m_currentCoredumpList.clear();
    malloc_trim(0);
}

void LogBackend::parse(LOG_FILTER_BASE &filter)
{
    m_type2ThreadIndex[filter.type] = m_logFileParser.parse(filter);
    m_type2Filter[filter.type] = filter;
}

void LogBackend::parseByJournal(const QStringList &arg)
{
    m_journalCurrentIndex = m_logFileParser.parseByJournal(arg);
}

void LogBackend::parseByJournalBoot(const QStringList &arg)
{
    m_journalBootCurrentIndex = m_logFileParser.parseByJournalBoot(arg);
}

void LogBackend::parseByDpkg(const DKPG_FILTERS &iDpkgFilter)
{
    m_dpkgCurrentIndex = m_logFileParser.parseByDpkg(iDpkgFilter);
}

void LogBackend::parseByXlog(const XORG_FILTERS &iXorgFilter)
{
    m_xorgCurrentIndex = m_logFileParser.parseByXlog(iXorgFilter);
}

void LogBackend::parseByBoot()
{
    m_bootCurrentIndex = m_logFileParser.parseByBoot();
}

void LogBackend::parseByKern(const KERN_FILTERS &iKernFilter)
{
    m_kernCurrentIndex = m_logFileParser.parseByKern(iKernFilter);
}

void LogBackend::parseByApp(const APP_FILTERS &iAPPFilter)
{
    m_appCurrentIndex = m_logFileParser.parseByApp(iAPPFilter);
}

void LogBackend::parseByDnf(DNF_FILTERS iDnfFilter)
{
    m_logFileParser.parseByDnf(iDnfFilter);
}

void LogBackend::parseByDmesg(DMESG_FILTERS iDmesgFilter)
{
    m_logFileParser.parseByDmesg(iDmesgFilter);
}

void LogBackend::parseByNormal(const NORMAL_FILTERS &iNormalFiler)
{
    m_normalCurrentIndex = m_logFileParser.parseByNormal(iNormalFiler);
}

void LogBackend::parseByKwin(const KWIN_FILTERS &iKwinfilter)
{
    m_kwinCurrentIndex = m_logFileParser.parseByKwin(iKwinfilter);
}

void LogBackend::parseByOOC(const QString &path)
{
    m_OOCCurrentIndex = m_logFileParser.parseByOOC(path);
}

void LogBackend::parseByAudit(const AUDIT_FILTERS &iAuditFilter)
{
    m_auditCurrentIndex = m_logFileParser.parseByAudit(iAuditFilter);
}

void LogBackend::parseByCoredump(const COREDUMP_FILTERS &iCoredumpFilter, bool parseMap)
{
    m_coredumpCurrentIndex = m_logFileParser.parseByCoredump(iCoredumpFilter, parseMap);
}

int LogBackend::loadSegementPage(int nSegementIndex, bool bReset/* = true*/)
{
    if(nSegementIndex == -1)
        return -1;

    if (bReset) {
        clearAllDatalist();
    } else {
        // 搜索结果存在各分段数据只有几条的情况，因此进行分段搜索加载时，不能清空历史数据
        if (m_type2LogDataOrigin[m_flag].size() > SEGEMENT_SIZE)
            m_type2LogDataOrigin[m_flag].clear();
        if (m_type2LogData[m_flag].size() > SEGEMENT_SIZE)
            m_type2LogData[m_flag].clear();
    }

    m_type2Filter[m_flag].segementIndex = nSegementIndex;
    parse(m_type2Filter[m_flag]);

    qCDebug(logBackend) << QString("load seagement index: %1").arg(nSegementIndex);
    return nSegementIndex;
}

int LogBackend::getNextSegementIndex(LOG_FLAG type, bool bNext/* = true*/)
{
    qint64 totalLineCount = 0;
    int nSegementIndex = -1;
    if (type == KERN) {
        QStringList filePaths = DLDBusHandler::instance(this)->getFileInfo("kern");
        for (auto file: filePaths) {
            totalLineCount += DLDBusHandler::instance(this)->getLineCount(file);
        }
    } else if (type == Kwin) {
        totalLineCount = DLDBusHandler::instance(this)->getLineCount(KWIN_TREE_DATA);
    }

    // 计算分段段数
    m_segementCount = static_cast<int>(totalLineCount / SEGEMENT_SIZE) + 1;
    
    qint64 currentLineCount = (m_type2Filter[type].segementIndex + (bNext ? 1 : 0)) * SEGEMENT_SIZE;

    if (totalLineCount > currentLineCount) {
        if (bNext)
            nSegementIndex = ++m_type2Filter[type].segementIndex;
        else if (currentLineCount > 0){
            nSegementIndex = --m_type2Filter[type].segementIndex;
        }
    }

    return nSegementIndex;
}

void LogBackend::exportLogData(const QString &filePath, const QStringList &strLabels)
{
    // 不是追加导出，则先清除原文件
    if (!m_bSegementExporting && QFile::exists(filePath)) {
        QFile::remove(filePath);
    }

    QFileInfo fi(filePath.left(filePath.lastIndexOf("/")));
    if (!fi.exists() || filePath.isEmpty()) {
        qWarning(logBackend) <<  QString("outdir:%1 is not exists.").arg(fi.absoluteFilePath());
        onExportResult(false);
        return;
    }
    if (!fi.isWritable()) {
        onExportResult(false);
        qCCritical(logBackend) <<  QString("outdir:%1 is not writable.").arg(fi.absoluteFilePath());
        return;
    }

    if (View != m_sessionType) {
        if (!hasMatchedData(m_flag)) {
            if (m_bSegementExporting) {
                // 分段导出时，某段未匹配到数据，也是正常的
                return;
            } else {
                qCWarning(logBackend) << "No matching data..";
                qApp->exit(-1);
                return;
            }
        }
    }

    QStringList labels = strLabels;
    if (labels.isEmpty())
        labels = getLabels(m_flag);

    if ((m_flag == KERN || m_flag == Kwin)) {
        // 初始化分段导出线程
        if (!m_pSegementExportThread) {
            m_pSegementExportThread = new LogSegementExportThread(this);
            m_pSegementExportThread->enableAppendWrite();
            // 导出到doc，最后一步写入文件耗时较长，总进度条需要额外+1
            if (filePath.endsWith(".doc"))
                m_pSegementExportThread->setTotalProcess(m_segementCount + 1);
            else
                m_pSegementExportThread->setTotalProcess(m_segementCount);
            connect(m_pSegementExportThread, &LogSegementExportThread::sigResult, this, &LogBackend::onExportResult);
            connect(m_pSegementExportThread, &LogSegementExportThread::sigProgress, this, &LogBackend::onExportProgress);
            connect(m_pSegementExportThread, &LogSegementExportThread::sigProcessFull, this, &LogBackend::onExportFakeCloseDlg);
            connect(this, &LogBackend::stopExport, m_pSegementExportThread, &LogSegementExportThread::stopImmediately);
            QThreadPool::globalInstance()->start(m_pSegementExportThread);
        }

        m_pSegementExportThread->setParameter(filePath, m_type2LogData[m_flag], labels, m_flag);
    } else {
        LogExportThread *exportThread = new LogExportThread(this);
        connect(exportThread, &LogExportThread::sigResult, this, &LogBackend::onExportResult);
        connect(exportThread, &LogExportThread::sigProgress, this, &LogBackend::onExportProgress);
        connect(exportThread, &LogExportThread::sigProcessFull, this, &LogBackend::onExportFakeCloseDlg);
        connect(this, &LogBackend::stopExport, exportThread, &LogExportThread::stopImmediately);

        // 分段导出需要激活追加导出标记
        exportThread->enableAppendExport(m_bSegementExporting);

        if (filePath.endsWith(".txt")) {
            switch (m_flag) {
            //根据导出日志类型执行正确的导出逻辑
            case JOURNAL:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(jList.count()));
                exportThread->exportToTxtPublic(filePath, jList, labels, m_flag);
                break;
            case BOOT_KLU:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(jBootList.count()));
                exportThread->exportToTxtPublic(filePath, jBootList, labels, JOURNAL);
                break;
            case APP: {
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(appList.count()));
                QString transAppName = LogApplicationHelper::instance()->transName(m_appFilter.app);
                exportThread->exportToTxtPublic(filePath, appList, labels, transAppName);
                break;
            }
            case DPKG:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dList.count()));
                exportThread->exportToTxtPublic(filePath, dList, labels);
                break;
            case BOOT:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(currentBootList.count()));
                exportThread->exportToTxtPublic(filePath, currentBootList, labels);
                break;
            case XORG:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(xList.count()));
                exportThread->exportToTxtPublic(filePath, xList, labels);
                break;
            case Normal:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(nortempList.count()));
                exportThread->exportToTxtPublic(filePath, nortempList, labels);
                break;
            case KERN:
            case Kwin:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(m_type2LogData[m_flag].count()));
                exportThread->exportToTxtPublic(filePath, m_type2LogData[m_flag], labels, m_flag);
                break;
            case Dmesg:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dmesgList.count()));
                exportThread->exportToTxtPublic(filePath, dmesgList, labels);
                break;
            case Dnf:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dnfList.count()));
                exportThread->exportToTxtPublic(filePath, dnfList, labels);
                break;
            case Audit:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(aList.count()));
                exportThread->exportToTxtPublic(filePath, aList, labels);
                break;
            default:
                break;
            }
            QThreadPool::globalInstance()->start(exportThread);
        } else if (filePath.endsWith(".html")) {
            switch (m_flag) {
            case JOURNAL:
                PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(jList.count()));
                exportThread->exportToHtmlPublic(filePath, jList, labels, m_flag);
                break;
            case BOOT_KLU:
                PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(jBootList.count()));
                exportThread->exportToHtmlPublic(filePath, jBootList, labels, JOURNAL);
                break;
            case APP: {
                PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(appList.count()));
                QString transAppName = LogApplicationHelper::instance()->transName(m_appFilter.app);
                exportThread->exportToHtmlPublic(filePath, appList, labels, transAppName);
                break;
            }
            case DPKG:
                PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(dList.count()));
                exportThread->exportToHtmlPublic(filePath, dList, labels);
                break;
            case BOOT:
                PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(currentBootList.count()));
                exportThread->exportToHtmlPublic(filePath, currentBootList, labels);
                break;
            case XORG:
                PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(xList.count()));
                exportThread->exportToHtmlPublic(filePath, xList, labels);
                break;
            case Normal:
                PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(nortempList.count()));
                exportThread->exportToHtmlPublic(filePath, nortempList, labels);
                break;
            case KERN:
            case Kwin:
                PERF_PRINT_BEGIN("POINT-04", QString("format=html count=%1").arg(m_type2LogData[m_flag].count()));
                exportThread->exportToHtmlPublic(filePath, m_type2LogData[m_flag], labels, m_flag);
                break;
            case Dmesg:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dmesgList.count()));
                exportThread->exportToHtmlPublic(filePath, dmesgList, labels);
                break;
            case Dnf:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dnfList.count()));
                exportThread->exportToHtmlPublic(filePath, dnfList, labels);
                break;
            case Audit:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(aList.count()));
                exportThread->exportToHtmlPublic(filePath, aList, labels);
                break;
            default:
                break;
            }
            QThreadPool::globalInstance()->start(exportThread);
        } else if (filePath.endsWith(".doc")) {
            switch (m_flag) {
            case JOURNAL:
                PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(jList.count()));
                exportThread->exportToDocPublic(filePath, jList, labels, m_flag);
                break;
            case BOOT_KLU:
                PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(jBootList.count()));
                exportThread->exportToDocPublic(filePath, jBootList, labels, JOURNAL);
                break;
            case APP: {
                PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(appList.count()));
                QString transAppName = LogApplicationHelper::instance()->transName(m_appFilter.app);
                exportThread->exportToDocPublic(filePath, appList, labels, transAppName);
                break;
            }
            case DPKG:
                PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(dList.count()));
                exportThread->exportToDocPublic(filePath, dList, labels);
                break;
            case BOOT:
                PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(currentBootList.count()));
                exportThread->exportToDocPublic(filePath, currentBootList, labels);
                break;
            case XORG:
                PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(xList.count()));
                exportThread->exportToDocPublic(filePath, xList, labels);
                break;
            case Normal:
                PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(nortempList.count()));
                exportThread->exportToDocPublic(filePath, nortempList, labels);
                break;
            case KERN:
            case Kwin:
                PERF_PRINT_BEGIN("POINT-04", QString("format=doc count=%1").arg(m_type2LogData[m_flag].count()));
                exportThread->exportToDocPublic(filePath, m_type2LogData[m_flag], labels, m_flag);
                break;
            case Dmesg:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dmesgList.count()));
                exportThread->exportToDocPublic(filePath, dmesgList, labels);
                break;
            case Dnf:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dnfList.count()));
                exportThread->exportToDocPublic(filePath, dnfList, labels);
                break;
            case Audit:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(aList.count()));
                exportThread->exportToDocPublic(filePath, aList, labels);
                break;
            default:
                break;
            }
            QThreadPool::globalInstance()->start(exportThread);
        } else if (filePath.endsWith(".xls")) {
            switch (m_flag) {
            case JOURNAL:
                PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(jList.count()));
                exportThread->exportToXlsPublic(filePath, jList, labels, m_flag);
                break;
            case BOOT_KLU:
                PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(jBootList.count()));
                exportThread->exportToXlsPublic(filePath, jBootList, labels, JOURNAL);
                break;
            case APP: {
                PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(appList.count()));
                QString transAppName = LogApplicationHelper::instance()->transName(m_appFilter.app);
                exportThread->exportToXlsPublic(filePath, appList, labels, transAppName);
                break;
            }
            case DPKG:
                PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(dList.count()));
                exportThread->exportToXlsPublic(filePath, dList, labels);
                break;
            case BOOT:
                PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(currentBootList.count()));
                exportThread->exportToXlsPublic(filePath, currentBootList, labels);
                break;
            case XORG:
                PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(xList.count()));
                exportThread->exportToXlsPublic(filePath, xList, labels);
                break;
            case Normal:
                PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(nortempList.count()));
                exportThread->exportToXlsPublic(filePath, nortempList, labels);
                break;
            case KERN:
            case Kwin:
                PERF_PRINT_BEGIN("POINT-04", QString("format=xls count=%1").arg(m_type2LogData[m_flag].count()));
                exportThread->exportToXlsPublic(filePath, m_type2LogData[m_flag], labels, m_flag);
                break;
            case Dmesg:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dmesgList.count()));
                exportThread->exportToXlsPublic(filePath, dmesgList, labels);
                break;
            case Dnf:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(dnfList.count()));
                exportThread->exportToXlsPublic(filePath, dnfList, labels);
                break;
            case Audit:
                PERF_PRINT_BEGIN("POINT-04", QString("format=txt count=%1").arg(aList.count()));
                exportThread->exportToXlsPublic(filePath, aList, labels);
                break;
            default:
                break;
            }
            QThreadPool::globalInstance()->start(exportThread);
        } else if (filePath.endsWith(".zip") && m_flag == COREDUMP) {
            PERF_PRINT_BEGIN("POINT-04", QString("format=zip count=%1").arg(m_currentCoredumpList.count()));
            exportThread->exportToZipPublic(filePath, m_currentCoredumpList, labels);
            QThreadPool::globalInstance()->start(exportThread);
        }
    }

    m_exportFilePath = filePath;

    qCInfo(logBackend) << "exporting ...";
}

void LogBackend::segementExport()
{
    if (m_flag != KERN && m_flag != Kwin)
        return;

    // 判断是否需要分段导出
    int nSegementIndex = getNextSegementIndex(m_flag);
    m_bSegementExporting = nSegementIndex != -1;

    // 解析下一段数据
    if (nSegementIndex != -1) {
        // 记录任务状态
        if (View == m_sessionType) {
            m_lastSessionType = m_sessionType;
            m_sessionType = Export;
        }
        loadSegementPage(nSegementIndex);
    } else {
        if (m_pSegementExportThread) {
            // 结束分段导出线程，保存数据到文件
            m_pSegementExportThread->stop();
            m_pSegementExportThread = nullptr;
        }

        // 分段导出完成，还原任务状态
        if (View == m_lastSessionType || View == m_sessionType) {
            m_sessionType = View;
            // 还原查看界面数据内容到导第一分段页
            emit clearTable();
            loadSegementPage(0);
        }
    }
}

void LogBackend::stopExportFromUI()
{
    emit stopExport();

    //若界面正在分段导出，则停止导出
    // TODO: 分段导出情况下，ExportThread::stopImmediately槽函数不能正常触发，暂时先这样修改
    if (Export == m_sessionType) {
        if (View == m_lastSessionType) {
            m_sessionType = View;
            // 线程在收到emit stopExport时，即调用stopImmediately接口停止运行，并自行析构
            // 因此只需对其置空, 不用delete
            if (m_pSegementExportThread) {
                m_pSegementExportThread = nullptr;
            }

            // 重置进度条
            emit sigProgress(0, 0);

            m_bSegementExporting = false;
            Utils::checkAndDeleteDir(m_exportFilePath);

            // 重置回第一分段页
            emit clearTable();
            loadSegementPage(0);
        }
    }
}

QList<QString> LogBackend::filterLog(const QString &iSearchStr, const QList<QString> &iList)
{
    QList<QString> rsList;
    if (iSearchStr.isEmpty())
        return iList;

    QJsonParseError parseError;
    for (auto data : iList) {
        QJsonDocument document = QJsonDocument::fromJson(data.toUtf8(), &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            if (document.isObject()) {
                QJsonObject object = document.object();
                for (auto it = object.constBegin(); it != object.end(); ++it) {
                    if (it.value().toString().contains(iSearchStr, Qt::CaseInsensitive))
                        rsList.append(data);
                }
            }
        }
    }

    return rsList;
}

BUTTONID LogBackend::period2Enum(const QString &period)
{
    BUTTONID id = INVALID;
    if (period == "all")
        id = ALL;
    else if (period == "today")
        id = ONE_DAY;
    else if (period == "3d")
        id = THREE_DAYS;
    else if (period == "1w")
        id = ONE_WEEK;
    else if (period == "1m")
        id = ONE_MONTH;
    else if (period == "3m")
        id = THREE_MONTHS;

    return id;
}

int LogBackend::level2Id(const QString &level)
{
    int lId = -2;
    if (level == "debug" || level == "7")
        lId = 7;
    if (level == "info" || level == "6")
        lId = 6;
    else if (level == "notice" || level == "5")
        lId = 5;
    else if (level == "warning" || level == "4")
        lId = 4;
    else if (level == "err" || level == "error" || level == "3")
        lId = 3;
    else if (level == "critical" || level == "crit" || level == "2")
        lId = 2;
    else if (level == "alert" || level == "1")
        lId = 1;
    else if (level == "emerg" || level == "0")
        lId = 0;
    else if (level == "all")
        lId = -1;
    else if (level.isEmpty())
        lId = -1;

    return lId;
}

DNFPRIORITY LogBackend::dnfLevel2Id(const QString &level)
{
    DNFPRIORITY eId = DNFINVALID;
    if (level == "trace" || level == "6")
        eId = TRACE;
    else if (level == "debug" || level == "5")
        eId = DEBUG;
    else if (level == "info" || level == "4")
        eId = INFO;
    else if (level == "warning" || level == "3")
        eId = WARNING;
    else if (level == "error" || level == "err" || level == "2")
        eId = ERROR;
    else if (level == "critical" || level == "crit" || level == "1")
        eId = CRITICAL;
    else if (level == "supercritical" || level == "supercrit" || level == "0")
        eId = SUPERCRITICAL;
    else if (level == "all")
        eId = DNFLVALL;
    else if (level.isEmpty())
        eId = DNFLVALL;

    return eId;
}

int LogBackend::normal2eventType(const QString &eventType)
{
    int type = -1;
    if (eventType == "all" || eventType == "0")
        type = 0;
    else if (eventType == "login" || eventType == "1")
        type = 1;
    else if (eventType == "boot" || eventType == "2")
        type = 2;
    else if (eventType == "shutdown" || eventType == "3")
        type = 3;

    return type;
}

int LogBackend::audit2eventType(const QString &eventType)
{
    int type = -1;
    if (eventType == "all" || eventType == "0")
        type = 0;
    else if (eventType == "identauth" || eventType == "1")
        type = 1;
    else if (eventType == "dac" || eventType == "2")
        type = 2;
    else if (eventType == "mac" || eventType == "3")
        type = 3;
    else if (eventType == "remote" || eventType == "4")
        type = 4;
    else if (eventType == "docaudit" || eventType == "5")
        type = 5;
    else if (eventType == "other" || eventType == "6")
        type = 6;

    return type;
}

TIME_RANGE LogBackend::getTimeRange(const BUTTONID &periodId)
{
    TIME_RANGE tr;

    QDateTime dtStart = QDateTime::currentDateTime();
    QDateTime dtEnd = dtStart;
    dtStart.setTime(QTime());
    dtEnd.setTime(QTime(23, 59, 59, 999));
    tr.end = dtEnd.toMSecsSinceEpoch();

    switch (periodId) {
    case ALL: {
        tr.begin = -1;
        tr.end = -1;
    }
    break;
    case ONE_DAY: {
        tr.begin = dtStart.toMSecsSinceEpoch();
    }
    break;
    case THREE_DAYS: {
        tr.begin = dtStart.addDays(-2).toMSecsSinceEpoch();
    }
    break;
    case ONE_WEEK: {
        tr.begin = dtStart.addDays(-6).toMSecsSinceEpoch();
    }
    break;
    case ONE_MONTH: {
        tr.begin = dtStart.addMonths(-1).toMSecsSinceEpoch();
        if (m_flag == Dmesg || m_flag == Dnf) {
            tr.begin = dtStart.addDays(-29).toMSecsSinceEpoch();
        }
    }
    break;
    case THREE_MONTHS: {
        tr.begin = dtStart.addMonths(-3).toMSecsSinceEpoch();
        if (m_flag == Dmesg || m_flag == Dnf) {
            tr.begin = dtStart.addDays(-89).toMSecsSinceEpoch();
        }
    }
    break;
    default: {
        tr.begin = -1;
        tr.end = -1;
    }
    break;
    }

    return tr;
}

QString LogBackend::getApplogPath(const QString &appName)
{
    QString logPath;
    QMap<QString, QString> appData = LogApplicationHelper::instance()->getMap();
    for (auto &it2 : appData.toStdMap()) {
        if (it2.second.contains(appName)) {
            logPath = it2.second;
            break;
        }
    }

    if (logPath.isEmpty())
        logPath = LogApplicationHelper::instance()->getPathByAppId(appName);

    return logPath;
}

QStringList LogBackend::getLabels(const LOG_FLAG &flag)
{
    QStringList labels = QStringList();
    switch (flag) {
    case JOURNAL: {
            labels << QCoreApplication::translate("Table", "Level")
                   << QCoreApplication::translate("Table", "Process") // modified by Airy
                   << QCoreApplication::translate("Table", "Date and Time")
                   << QCoreApplication::translate("Table", "Info")
                   << QCoreApplication::translate("Table", "User")
                   << QCoreApplication::translate("Table", "PID");
    }
    break;
    case Dmesg: {
            labels  << QCoreApplication::translate("Table", "Level")
                    << QCoreApplication::translate("Table", "Date and Time")
                    << QCoreApplication::translate("Table", "Info");
    }
    break;
    case KERN: {
            labels << QCoreApplication::translate("Table", "Date and Time")
                   << QCoreApplication::translate("Table", "User")
                   << QCoreApplication::translate("Table", "Process")
                   << QCoreApplication::translate("Table", "Info");
    }
    break;
    case BOOT_KLU: {
            labels << QCoreApplication::translate("Table", "Level")
                   << QCoreApplication::translate("Table", "Process")
                   << QCoreApplication::translate("Table", "Date and Time")
                   << QCoreApplication::translate("Table", "Info")
                   << QCoreApplication::translate("Table", "User")
                   << QCoreApplication::translate("Table", "PID");
    }
    break;
    case BOOT: {
            labels << QCoreApplication::translate("Table", "Status")
                   << QCoreApplication::translate("Table", "Info");
    }
    break;
    case DPKG: {
            labels << QCoreApplication::translate("Table", "Date and Time")
                   << QCoreApplication::translate("Table", "Info")
                   << QCoreApplication::translate("Table", "Action");
    }
    break;
    case Dnf: {
            labels << QCoreApplication::translate("Table", "Level")
                   << QCoreApplication::translate("Table", "Date and Time")
                   << QCoreApplication::translate("Table", "Info");
    }
    break;
    case Kwin: {
            labels << QCoreApplication::translate("Table", "Info");
    }
    break;
    case XORG: {
            labels << QCoreApplication::translate("Table", "Offset")
                   << QCoreApplication::translate("Table", "Info");
    }
    break;
    case APP: {
            labels << QCoreApplication::translate("Table", "Level")
                   << QCoreApplication::translate("Table", "Date and Time")
                   << QCoreApplication::translate("Table", "Source")
                   << QCoreApplication::translate("Table", "Info");
    }
    break;
    case COREDUMP: {
            labels << QCoreApplication::translate("Table", "SIG")
                   << QCoreApplication::translate("Table", "Date and Time")
                   << QCoreApplication::translate("Table", "Core File")
                   << QCoreApplication::translate("Table", "User Name ")
                   << QCoreApplication::translate("Table", "EXE");
    }
    break;
    case Normal: {
            labels << QCoreApplication::translate("Table", "Event Type")
                   << QCoreApplication::translate("Table", "Username")
                   << QCoreApplication::translate("Tbble", "Date and Time")
                   << QCoreApplication::translate("Table", "Info");
    }
    break;
    case Audit: {
            labels << QCoreApplication::translate("Table", "Event Type")
                   << QCoreApplication::translate("Table", "Date and Time")
                   << QCoreApplication::translate("Table", "Process")
                   << QCoreApplication::translate("Table", "Status")
                   << QCoreApplication::translate("Table", "Info");
    }
    break;
    default:
    break;
    }

    return labels;
}

bool LogBackend::hasMatchedData(const LOG_FLAG &flag)
{
    bool bMatchedData = false;
    switch (flag) {
    case JOURNAL: {
        if (!jList.isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    case Dmesg: {
        if (!dmesgList.isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    case Kwin:
    case KERN: {
        if (!m_type2LogData[flag].isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    case BOOT_KLU: {
        if (!jBootList.isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    case BOOT: {
        if (!currentBootList.isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    case DPKG: {
        if (!dList.isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    case Dnf: {
        if (!dnfList.isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    case XORG: {
        if (!xList.isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    case APP: {
        if (!appList.isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    case COREDUMP: {
        if (!m_currentCoredumpList.isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    case Normal: {
        if (!nortempList.isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    case Audit: {
        if (!aList.isEmpty()) {
            bMatchedData = true;
        }
    }
    break;
    default:
    break;
    }

    return bMatchedData;
}

void LogBackend::parseCoredumpDetailInfo(QList<LOG_MSG_COREDUMP> &list)
{
    QProcess process;
    for (auto &data : list) {
        QDateTime coredumpTime = QDateTime::fromString(data.dateTime, "yyyy-MM-dd hh:mm:ss");
        // 解析coredump文件保存位置
        if (data.coreFile != "missing") {
            QString outInfoByte;
            // get maps info
            const QString &corePath = QDir::tempPath() + QString("/%1.dump").arg(QFileInfo(data.storagePath).fileName());
            DLDBusHandler::instance()->readLog(QString("coredumpctl dump %1 -o %2").arg(data.pid).arg(corePath));
            outInfoByte = DLDBusHandler::instance()->readLog(QString("readelf -n %1").arg(corePath));

            data.maps = outInfoByte;

            // 获取二进制文件信息
            process.start("/bin/bash", QStringList() << "-c" << QString("file %1").arg(data.exe));
            process.waitForFinished(-1);
            outInfoByte = process.readAllStandardOutput();
            if (!outInfoByte.isEmpty())
                data.binaryInfo = outInfoByte;

            // 获取包名
            process.start("/bin/bash", QStringList() << "-c" << QString("dpkg -S %1").arg(data.exe));
            process.waitForFinished(-1);
            outInfoByte = process.readAllStandardOutput();
            // 获取版本号
            if (!outInfoByte.isEmpty()) {
                QString str = outInfoByte;
                process.start("/bin/bash", QStringList() << "-c" << QString("dpkg-query --show %1").arg(str.split(":").first()));
                process.waitForFinished(-1);
                outInfoByte = process.readAllStandardOutput();
                if (!outInfoByte.isEmpty()) {
                    data.packgeVersion = QString(outInfoByte).simplified();
                }
            }
        }

        // 若为窗管崩溃，提取窗管最后100行日志到coredump信息中
        if (data.exe == KWAYLAND_EXE_PATH || data.exe == XWAYLAND_EXE_PATH) {
            // 窗管日志存放在用户家目录下，因此根据崩溃信息所属用户id获取用户家目录
            uint userId = data.uid.toUInt();
            QString userHomePath = Utils::getUserHomePathByUID(userId);
            if (!userHomePath.isEmpty()) {
                if (data.exe == KWAYLAND_EXE_PATH) {
                    data.appLog = DLDBusHandler::instance()->readLogLinesInRange(QString("%1/.kwin-old.log").arg(userHomePath), 0, KWIN_LASTLINE_NUM).join('\n');
                    if (!data.appLog.isEmpty())
                        qCInfo(logBackend) << QString("kwin crash log:%1").arg(data.appLog);
                } else if (data.exe == XWAYLAND_EXE_PATH) {
                    data.appLog = DLDBusHandler::instance()->readLogLinesInRange(QString("%1/.xwayland.log.old").arg(userHomePath), KWIN_LASTLINE_NUM).join('\n');
                }
            } else {
                qCWarning(logBackend) << QString("uid:%1 homepath is empty.").arg(userId);
            }
        }
    }
}
