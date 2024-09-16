// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logauththread.h"
#include "utils.h"
#include "sharedmemorymanager.h"
#include "sys/utsname.h"
#include "wtmpparse.h"
#include "dbusproxy/dldbushandler.h"
#include "dbusmanager.h"

#include <DGuiApplicationHelper>
#include <DApplication>

#include <QDebug>
#include <QDateTime>
#include <time.h>
#include <utmp.h>
#include <utmpx.h>
#include <algorithm>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <iostream>
#include <fstream>
using namespace std;

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logAuthWork, "org.deepin.log.viewer.auth.work")
#else
Q_LOGGING_CATEGORY(logAuthWork, "org.deepin.log.viewer.auth.work", QtInfoMsg)
#endif

DGUI_USE_NAMESPACE
int LogAuthThread::thread_count = 0;

// 崩溃信号列表对应字符值
const QStringList sigList = { "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1",
        "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM", "SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP",
        "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGIO", "SIGPWR", "SIGSYS"};

// DBUS传输文件大小阈值 100MB
#define DBUS_THRESHOLD_MAX 100

/**
 * @brief LogAuthThread::LogAuthThread 构造函数
 * @param parent 父对象
 */
LogAuthThread::LogAuthThread(QObject *parent)
    :  QObject(parent)
    , QRunnable()
    , m_type(NONE)
{
    //使用线程池启动该线程，跑完自己删自己
    setAutoDelete(true);
    //静态计数变量加一并赋值给本对象的成员变量，以供外部判断是否为最新线程发出的数据信号
    thread_count++;
    m_threadCount = thread_count;
    initDnfLevelMap();
    initLevelMap();
}

/**
 * @brief LogAuthThread::~LogAuthThread 析构函数，停止并销毁process指针
 */
LogAuthThread::~LogAuthThread()
{
    stopProccess();
}
void LogAuthThread::initDnfLevelMap()
{
    m_dnfLevelDict.insert("TRACE", TRACE);
    m_dnfLevelDict.insert("SUBDEBUG", DEBUG);
    m_dnfLevelDict.insert("DDEBUG", DEBUG);
    m_dnfLevelDict.insert("DEBUG", DEBUG);
    m_dnfLevelDict.insert("INFO", INFO);
    m_dnfLevelDict.insert("WARNING", WARNING);
    m_dnfLevelDict.insert("ERROR", ERROR);
    m_dnfLevelDict.insert("CRITICAL", CRITICAL);
    m_dnfLevelDict.insert("SUPERCRITICAL", SUPERCRITICAL);

    m_transDnfDict.insert("TRACE", Dtk::Widget::DApplication::translate("Level", "Trace"));
    m_transDnfDict.insert("SUBDEBUG", Dtk::Widget::DApplication::translate("Level", "Debug"));
    m_transDnfDict.insert("DDEBUG", Dtk::Widget::DApplication::translate("Level", "Debug"));
    m_transDnfDict.insert("DEBUG", Dtk::Widget::DApplication::translate("Level", "Debug"));
    m_transDnfDict.insert("INFO", Dtk::Widget::DApplication::translate("Level", "Info"));
    m_transDnfDict.insert("WARNING", Dtk::Widget::DApplication::translate("Level", "Warning"));
    m_transDnfDict.insert("ERROR", Dtk::Widget::DApplication::translate("Level", "Error"));
    m_transDnfDict.insert("CRITICAL", Dtk::Widget::DApplication::translate("Level", "Critical"));
    m_transDnfDict.insert("SUPERCRITICAL", Dtk::Widget::DApplication::translate("Level", "Super critical"));
}

void LogAuthThread::initLevelMap()
{
    m_levelMap.clear();
    m_levelMap.insert(0, Dtk::Widget::DApplication::translate("Level", "Emergency"));
    m_levelMap.insert(1, Dtk::Widget::DApplication::translate("Level", "Alert"));
    m_levelMap.insert(2, Dtk::Widget::DApplication::translate("Level", "Critical"));
    m_levelMap.insert(3, Dtk::Widget::DApplication::translate("Level", "Error"));
    m_levelMap.insert(4, Dtk::Widget::DApplication::translate("Level", "Warning"));
    m_levelMap.insert(5, Dtk::Widget::DApplication::translate("Level", "Notice"));
    m_levelMap.insert(6, Dtk::Widget::DApplication::translate("Level", "Info"));
    m_levelMap.insert(7, Dtk::Widget::DApplication::translate("Level", "Debug"));
}

/**
 * @brief LogAuthThread::stopProccess 停止日志数据获取进程并销毁
 */
void LogAuthThread::stopProccess()
{
    //防止正在执行时重复执行
    if (m_isStopProccess) {
        return;
    }
    m_isStopProccess = true;
    //停止获取线程执行，标记量置false
    m_canRun = false;
    if (!Utils::runInCmd) {
        //共享内存数据结构，用于和获取进程共享内存，数据为是否可执行进程，用于控制数据获取进程停止，因为这里会出现需要提权执行的进程，主进程没有权限停止提权进程，所以使用共享内存变量标记量控制提权进程停止
        ShareMemoryInfo   shareInfo ;
        //设置进程为不可执行
        shareInfo.isStart = false;
        //把数据付给共享内存中对应的变量
        SharedMemoryManager::instance()->setRunnableTag(shareInfo);
    }
    if (m_process) {
        m_process->kill();
    }
}

void LogAuthThread::setFilePath(const QStringList &filePath)
{
    m_FilePath = filePath;
}

int LogAuthThread::getIndex()
{
    return  m_threadCount;
}
QString LogAuthThread::startTime()
{
    QString startStr = "";
    QFile startFile("/proc/uptime");
    if (!startFile.exists()) {
        return "";
    }
    if (startFile.open(QFile::ReadOnly)) {
        startStr = QString(startFile.readLine());
        startFile.close();
    }

    startStr = startStr.split(" ").value(0, "");
    if (startStr.isEmpty()) {
        return "";
    }
    return startStr;
}

/**
 * @brief LogAuthThread::run 线程执行虚函数
 */
void LogAuthThread::run()
{
    //此线程刚开始把可以继续变量置true，不然下面没法跑
    m_canRun = true;
    //根据类型成员变量执行对应日志的获取逻辑
    switch (m_type) {
    case KERN:
        handleKern();
        break;
    case BOOT:
        handleBoot();
        break;
    case Kwin:
        handleKwin();
        break;
    case XORG:
        handleXorg();
        break;
    case DPKG:
        handleDkpg();
        break;
    case Normal:
        handleNormal();
        break;
    case Dnf:
        handleDnf();
        break;
    case Dmesg:
        handleDmesg();
        break;
    case Audit:
        handleAudit();
        break;
    case COREDUMP:
        handleCoredump();
        break;
    default:
        break;
    }

    m_canRun = false;
}

#include <QFile>
/**
 * @brief LogAuthThread::handleBoot 处理启动日志
 */
void LogAuthThread::handleBoot()
{
    QList<LOG_MSG_BOOT> bList;
    for (int i = 0; i < m_FilePath.count(); i++) {
        if (!m_FilePath.at(i).contains("txt")) {
            QFile file(m_FilePath.at(i)); // add by Airy
            if (!file.exists()) {
                emit bootFinished(m_threadCount);
                return;
            }
        }
        if (!m_canRun) {
            return;
        }

        if (!Utils::runInCmd) {
            initProccess();
            m_process->setProcessChannelMode(QProcess::MergedChannels);
            //共享内存对应变量置true，允许进程内部逻辑运行
            ShareMemoryInfo shareInfo;
            shareInfo.isStart = true;
            SharedMemoryManager::instance()->setRunnableTag(shareInfo);
            //启动日志需要提权获取，运行的时候把对应共享内存的名称传进去，方便获取进程拿标记量判断是否继续运行
            m_process->start("pkexec", QStringList() << "logViewerAuth"
                             << m_FilePath.at(i) << SharedMemoryManager::instance()->getRunnableKey());
            m_process->waitForFinished(-1);
            if (m_process->exitCode() != 0) {
                emit bootFinished(m_threadCount);
                return;
            }
        }

        QString byte = DLDBusHandler::instance(this)->readLog(m_FilePath.at(i));
        byte.replace('\u0000', "").replace("\x01", "");
        QStringList strList = byte.split('\n', QString::SkipEmptyParts);

        //按换行分割
        for (int j = strList.size() - 1; j >= 0; --j) {
            QString lineStr = strList.at(j);
            if (lineStr.startsWith("/dev") || lineStr.isEmpty())
                continue;
            //删除颜色格式字符
            lineStr.replace(QRegExp("\\#033\\[\\d+(;\\d+){0,2}m"), "");
            // remove Useless characters
            lineStr.replace(QRegExp("\\x1B\\[\\d+(;\\d+){0,2}m"), "");
            Utils::replaceColorfulFont(&lineStr);
            QStringList retList;
            LOG_MSG_BOOT bMsg;
            retList = lineStr.split(" ", QString::SkipEmptyParts);
            if (retList.size() == 1) {
                bMsg.msg = lineStr.trimmed();
                bList.append(bMsg);
            } else {
                if (retList[1].compare("OK", Qt::CaseInsensitive) == 0 || retList[1].compare("Failed", Qt::CaseInsensitive) == 0) {
                    //如果是以ok/failed开头，则认为是一条记录的开头，否则认为此行为上一条日志的信息体的后续
                    bMsg.status = retList[1];
                    QStringList leftList = retList.mid(3);
                    bMsg.msg += leftList.join(" ");
                    bList.append(bMsg);
                } else {
                    bMsg.msg = lineStr.trimmed();
                    bList.append(bMsg);
                }
            }

            //每获得500个数据就发出信号给控件加载
            if (bList.count() % SINGLE_READ_CNT == 0) {
                emit bootData(m_threadCount, bList);
                bList.clear();
            }
        }
    }
    //最后可能有余下不足500的数据
    if (bList.count() >= 0) {
        emit bootData(m_threadCount, bList);
    }
    emit bootFinished(m_threadCount);
}

/**
 * @brief LogAuthThread::handleKern 处理获取内核日志
 */
void LogAuthThread::handleKern()
{
    QList<LOG_MSG_JOURNAL> kList;
    for (int i = 0; i < m_FilePath.count(); i++) {
        if (!m_FilePath.at(i).contains("txt")) {
            QFile file(m_FilePath.at(i)); // add by Airy
            if (!file.exists()) {
                emit kernFinished(m_threadCount);
                return;
            }
        }
        if (!m_canRun) {
            return;
        }

        if (!Utils::runInCmd) {
            initProccess();
            if (!m_canRun) {
                return;
            }
            m_process->setProcessChannelMode(QProcess::MergedChannels);
            if (!m_canRun) {
                return;
            }
            //共享内存对应变量置true，允许进程内部逻辑运行
            ShareMemoryInfo shareInfo;
            shareInfo.isStart = true;
            SharedMemoryManager::instance()->setRunnableTag(shareInfo);
            //启动日志需要提权获取，运行的时候把对应共享内存的名称传进去，方便获取进程拿标记量判断是否继续运行
            m_process->start("pkexec", QStringList() << "logViewerAuth"
                             << m_FilePath.at(i) << SharedMemoryManager::instance()->getRunnableKey());
            m_process->waitForFinished(-1);
            //有错则传出空数据
            if (m_process->exitCode() != 0) {
                emit kernFinished(m_threadCount);
                return;
            }
        }

        if (!m_canRun) {
            return;
        }

        //如果是压缩文件，对其解压缩
        QString filePath = m_FilePath.at(i);
        if(QString::compare(QFileInfo(filePath).suffix(), "gz", Qt::CaseInsensitive) == 0){
            QStringList filePathList = DLDBusHandler::instance(this)->getFileInfo(filePath);
            if(filePathList.size()){
                filePath = filePathList.at(0);
            }else {
                filePath = "";
            }
        }

        auto token = DLDBusHandler::instance(this)->openLogStream(filePath);
        QString byte;
        while(1) {
            auto temp = DLDBusHandler::instance(this)->readLogInStream(token);

            if(temp.isEmpty()) {
                break;
            }

            byte += temp;
        }

        byte.replace('\u0000', "").replace("\x01", "");
        QStringList strList = byte.split('\n', QString::SkipEmptyParts);
        for (int j = strList.size() - 1; j >= 0; --j) {
            if (!m_canRun) {
                return;
            }
            QString str = strList.at(j);
            LOG_MSG_JOURNAL msg;
            //删除颜色格式字符
            str.replace(QRegExp("\\#033\\[\\d+(;\\d+){0,2}m"), "");
            QStringList list = str.split(" ", QString::SkipEmptyParts);
            if (list.size() < 5)
                continue;
            //获取内核年份接口已添加，等待系统接口添加年份改变相关日志
            QStringList timeList;
            if (list[0].contains("-")) {
                timeList.append(list[0]);
                timeList.append(list[1]);
                iTime = formatDateTime(list[0], list[1]);
            } else {
                timeList.append(list[0]);
                timeList.append(list[1]);
                timeList.append(list[2]);
                iTime = formatDateTime(list[0], list[1], list[2]);
            }

            //对时间筛选
            if (m_kernFilters.timeFilterBegin > 0 && m_kernFilters.timeFilterEnd > 0) {
                if (iTime < m_kernFilters.timeFilterBegin || iTime > m_kernFilters.timeFilterEnd)
                    continue;
            }

            msg.dateTime = timeList.join(" ");
            QStringList tmpList;
            if (list[0].contains("-")) {
                msg.hostName = list[2];
                tmpList = list[3].split("[");
            } else {
                msg.hostName = list[3];
                tmpList = list[4].split("[");
            }

            int m = 0;
            //内核日志存在年份，解析用户名和进程id
            if (list[0].contains("-")) {
                if (tmpList.size() != 2) {
                    msg.daemonName = list[3].split(":")[0];
                } else {
                    msg.daemonName = list[3].split("[")[0];
                    QString id = list[3].split("[")[1];
                    id.chop(2);
                    msg.daemonId = id;
                }
                m = 4;
            } else {//内核日志不存在年份,解析用户名和进程id
                if (tmpList.size() != 2) {
                    msg.daemonName = list[4].split(":")[0];
                } else {
                    msg.daemonName = list[4].split("[")[0];
                    QString id = list[4].split("[")[1];
                    id.chop(2);
                    msg.daemonId = id;
                }
                m = 5;
            }

            QString msgInfo;
            for (int k = m; k < list.size(); k++) {
                msgInfo.append(list[k] + " ");
            }
            msg.msg = msgInfo;

            //            kList.append(msg);
            kList.append(msg);
            if (!m_canRun) {
                return;
            }
            //每获得500个数据就发出信号给控件加载
            if (kList.count() % SINGLE_READ_CNT == 0) {
                emit kernData(m_threadCount, kList);
                kList.clear();
            }
            if (!m_canRun) {
                return;
            }
        }
    }
    //最后可能有余下不足500的数据
    if (kList.count() >= 0) {
        emit kernData(m_threadCount, kList);
    }
    emit kernFinished(m_threadCount);
}

/**
 * @brief LogAuthThread::handleKwin 获取kwin日志逻辑
 */
void LogAuthThread::handleKwin()
{
    QFile file(KWIN_TREE_DATA);
    if (!m_canRun) {
        return;
    }
    QList<LOG_MSG_KWIN> kwinList;
    if (!file.exists()) {
        emit kwinFinished(m_threadCount);
        return;
    }
    if (!m_canRun) {
        return;
    }
    initProccess();
    m_process->start("cat", QStringList() << KWIN_TREE_DATA);
    m_process->waitForFinished(-1);
    if (!m_canRun) {
        return;
    }
    QByteArray outByte = m_process->readAllStandardOutput();
    if (!m_canRun) {
        return;
    }

    QStringList strList =  QString(Utils::replaceEmptyByteArray(outByte)).split('\n', QString::SkipEmptyParts);

    for (int i = strList.size() - 1; i >= 0 ; --i)  {
        QString str = strList.at(i);
        if (!m_canRun) {
            return;
        }
        if (str.trimmed().isEmpty()) {
            continue;
        }
        LOG_MSG_KWIN kwinMsg;
        kwinMsg.msg = str;
        kwinList.append(kwinMsg);
        //每获得500个数据就发出信号给控件加载
        if (kwinList.count() % SINGLE_READ_CNT == 0) {
            emit kwinData(m_threadCount, kwinList);
            kwinList.clear();
        }
    }

    if (!m_canRun) {
        return;
    }
    //最后可能有余下不足500的数据
    if (kwinList.count() >= 0) {
        emit kwinData(m_threadCount, kwinList);
    }
    emit kwinFinished(m_threadCount);
}

/**
 * @brief LogAuthThread::handleXorg 处理xorg日志获取逻辑
 */
void LogAuthThread::handleXorg()
{
    QList<LOG_MSG_XORG> xList;
    for (int i = 0; i < m_FilePath.count(); i++) {
        if (!m_FilePath.at(i).contains("txt")) {
            QFile file(m_FilePath.at(i)); // add by Airy
            if (!file.exists()) {
                emit proccessError(tr("Log file is empty"));
                emit xorgFinished(m_threadCount);
                return;
            }
        }
        if (!m_canRun) {
            return;
        }
        QString m_Log = DLDBusHandler::instance(this)->readLog(m_FilePath.at(i));
        QByteArray outByte = m_Log.toUtf8();
        if (!m_canRun) {
            return;
        }
        if (!m_canRun) {
            return;
        }
        QStringList fileInfoList = QString(Utils::replaceEmptyByteArray(outByte)).split('\n', QString::SkipEmptyParts);
        QString tempStr = "";
        for (QStringList::Iterator k = fileInfoList.end() - 1; k != fileInfoList.begin() - 1; --k) {
            QString &str = *k;
            //清除颜色格式字符
            str.replace(QRegExp("\\x1B\\[\\d+(;\\d+){0,2}m"), "");
            if (str.startsWith("[")) {
                QStringList list = str.split("]", QString::SkipEmptyParts);
                if (list.count() < 2)
                    continue;
                QString timeStr = list[0];
                QString msgInfo = list.mid(1, list.length() - 1).join("]").trimmed();
                // 仅显示时间偏移量（单位：秒
                QString tStr = timeStr.split("[", QString::SkipEmptyParts)[0].trimmed();
                LOG_MSG_XORG msg;
                msg.offset = tStr;
                msg.msg = msgInfo + tempStr;
                tempStr.clear();
                xList.append(msg);
                //每获得500个数据就发出信号给控件加载
                if (xList.count() % SINGLE_READ_CNT == 0) {
                    emit xorgData(m_threadCount, xList);
                    xList.clear();
                }
            } else {
                tempStr.prepend(" " + str);
            }
        }
    }
    if (!m_canRun) {
        return;
    }
    //最后可能有余下不足500的数据
    if (xList.count() >= 0) {
        emit xorgData(m_threadCount, xList);
    }
    emit xorgFinished(m_threadCount);
}

/**
 * @brief LogAuthThread::handleDkpg 获取dpkg逻辑
 */
void LogAuthThread::handleDkpg()
{
    QList<LOG_MSG_DPKG> dList;
    for (int i = 0; i < m_FilePath.count(); i++) {
        if (!m_FilePath.at(i).contains("txt")) {
            QFile file(m_FilePath.at(i)); // if not,maybe crash
            if (!file.exists())
                return;
        }
        if (!m_canRun) {
            return;
        }

        QString m_Log = DLDBusHandler::instance(this)->readLog(m_FilePath.at(i));
        QByteArray outByte = m_Log.toUtf8();
        if (!m_canRun) {
            return;
        }
        QStringList strList = QString(Utils::replaceEmptyByteArray(outByte)).split('\n', QString::SkipEmptyParts);
        for (int j = strList.size() - 1; j >= 0; --j) {
            QString str = strList.at(j);
            if (!m_canRun) {
                return;
            }
            str.replace(QRegExp("\\x1B\\[\\d+(;\\d+){0,2}m"), "");
            QStringList m_strList = str.split(" ", QString::SkipEmptyParts);
            if (m_strList.size() < 3)
                continue;

            QString info;
            for (auto k = 3; k < m_strList.size(); k++) {
                info = info + m_strList[k] + " ";
            }

            LOG_MSG_DPKG dpkgLog;
            dpkgLog.dateTime = m_strList[0] + " " + m_strList[1];
            QDateTime dt = QDateTime::fromString(dpkgLog.dateTime, "yyyy-MM-dd hh:mm:ss");
            //筛选时间
            if (m_dkpgFilters.timeFilterBegin > 0 && m_dkpgFilters.timeFilterEnd > 0) {
                if (dt.toMSecsSinceEpoch() < m_dkpgFilters.timeFilterBegin || dt.toMSecsSinceEpoch() > m_dkpgFilters.timeFilterEnd)
                    continue;
            }
            dpkgLog.action = m_strList[2];
            dpkgLog.msg = info;

            //        dList.append(dpkgLog);
            dList.append(dpkgLog);
            if (!m_canRun) {
                return;
            }
            //每获得500个数据就发出信号给控件加载
            if (dList.count() % SINGLE_READ_CNT == 0) {
                emit dpkgData(m_threadCount, dList);
                dList.clear();
            }
            if (!m_canRun) {
                return;
            }
        }
    }

    //最后可能有余下不足500的数据
    if (dList.count() >= 0) {
        emit dpkgData(m_threadCount, dList);
    }
    emit dpkgFinished(m_threadCount);
}

void LogAuthThread::handleNormal()
{
    if (!m_canRun) {
        emit normalFinished(m_threadCount);
        return;
    }

    struct utmp *utbufp;
    if (wtmp_open(QString(WTMP_FILE).toLatin1().data()) == -1) {
        printf("open WTMP_FILE file error\n");
        return;  // exit(1) will exit this application
    }

    NormalInfoTime();

    if (!m_canRun) {
        return;
    }

    int count1 = 0;
    QString a_name = "root";
    QLocale locale = QLocale::English;
    QList<LOG_MSG_NORMAL> nList;
    while ((utbufp = wtmp_next()) != (static_cast<struct utmp *>(nullptr))) {
        if (!m_canRun) {
            return;
        }
        if (utbufp->ut_type == RUN_LVL || utbufp->ut_type == BOOT_TIME || utbufp->ut_type == USER_PROCESS) {
            QString strtmp = utbufp->ut_name;
            if (strtmp.compare("runlevel") == 0 || (utbufp->ut_type == RUN_LVL && strtmp != "shutdown") || utbufp->ut_type == INIT_PROCESS || utbufp->ut_time <= 0) { // clear the runlevel
                continue;
            }

            LOG_MSG_NORMAL Nmsg;
            if (utbufp->ut_type == USER_PROCESS) {
                Nmsg.eventType = "Login";
                Nmsg.userName = utbufp->ut_name;
                a_name = Nmsg.userName;
            } else {
                Nmsg.eventType = utbufp->ut_name;
                if (strtmp.compare("reboot") == 0) {
                    Nmsg.eventType = "Boot";
                }
                Nmsg.userName = a_name;
            }
            if (Nmsg.eventType.compare("Login", Qt::CaseInsensitive) == 0) {
                Nmsg.eventType = "Login";
            }

            QString strFormat = "ddd MMM dd hh:mm";

            //修改时间格式转换方法，采用QDateTime 转换
            QString start_str = locale.toString(QDateTime::fromTime_t(static_cast<uint>(utbufp->ut_time)), strFormat);
            //截止时间解析
            if (Nmsg.eventType == "Login" || Nmsg.eventType == "Boot") {
                if (count1 <= TimeList.length() - 1) {
                    Nmsg.msg = TimeList[count1];
                    count1++;
                }
            } else {
                Nmsg.msg = start_str + "  -  ";
            }

            QString n_time = QDateTime::fromTime_t(static_cast<uint>(utbufp->ut_time)).toString("yyyy-MM-dd hh:mm:ss");
            Nmsg.dateTime = n_time;
            QDateTime nn_time = QDateTime::fromString(Nmsg.dateTime, "yyyy-MM-dd hh:mm:ss");
            if (m_normalFilters.timeFilterEnd > 0 && m_normalFilters.timeFilterBegin > 0) {
                if (nn_time.toMSecsSinceEpoch() < m_normalFilters.timeFilterBegin || nn_time.toMSecsSinceEpoch() > m_normalFilters.timeFilterEnd) { // add by Airy
                    continue;
                }
            }
            //last -f /var/log/wtmp
            nList.insert(0, Nmsg);
        }
    }
    wtmp_close();

    if (nList.count() >= 0) {
        emit normalData(m_threadCount, nList);
    }
    emit normalFinished(m_threadCount);
}

void LogAuthThread::NormalInfoTime()
{
    if (!m_canRun) {
        return;
    }
    initProccess();

    if (!m_canRun) {
        return;
    }
    //共享内存对应变量置true，允许进程内部逻辑运行
    ShareMemoryInfo shareInfo;
    shareInfo.isStart = true;
    SharedMemoryManager::instance()->setRunnableTag(shareInfo);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    m_process->start("last", {"-f", "/var/log/wtmp"});
    m_process->waitForFinished(-1);
    QByteArray outByte = m_process->readAllStandardOutput();
    QByteArray byte = Utils::replaceEmptyByteArray(outByte);
    QTextStream stream(&byte);
    QByteArray encode;
    stream.setCodec(encode);
    stream.readAll();
    QStringList l = QString(byte).split('\n');
    m_process->close();
    TimeList.clear();
    if (!m_canRun) {
        return;
    }
    for (QString str : l) {
        if (!m_canRun) {
            return;
        }
        if (str == "") continue;
        QString list = str.simplified();
        if (list == "") continue;
        int spacepos = list.indexOf(" ");
        QString name = list.left(spacepos);
        int spacepos2 = list.indexOf(" ", spacepos + 1);
        int spacepos3 = list.indexOf(" ", spacepos2 + 1);
        QString time1 = list.mid(spacepos3 + 1);
        int spacepos4 = list.indexOf(" ", spacepos3 + 1);
        QString time2 = list.mid(spacepos4 + 1);

        if (name == "wtmp") continue;
        if (name != "reboot" && name != "wtmp") {
            TimeList << time1;
        } else  if (name == "reboot") {
            TimeList << time2;
        }
    }
    std::reverse(TimeList.begin(), TimeList.end());
}

void LogAuthThread::handleDnf()
{
    QList<LOG_MSG_DNF> dList;
    for (int i = 0; i < m_FilePath.count(); i++) {
        if (!m_FilePath.at(i).contains("txt")) {
            QFile file(m_FilePath.at(i)); // if not,maybe crash
            if (!file.exists())
                return;
        }
        if (!m_canRun) {
            return;
        }
        QByteArray outByte = DLDBusHandler::instance(this)->readLog(m_FilePath.at(i)).toUtf8();
        QString output = Utils::replaceEmptyByteArray(outByte);
        QStringList allLog = output.split('\n');
        //dnf日志数据结构
        LOG_MSG_DNF dnfLog;
        //多行多余信息
        QString multiLine;
        //开启贪婪匹配，解析dnf全部字段:日期+事件+等级+主要内容
        QRegularExpression re("^(\\d{4}-[0-2]\\d-[0-3]\\d)\\D*([0-2]\\d:[0-5]\\d:[0-5]\\d)\\S*\\s*(\\w*)\\s*(.*)$");
        for (int j = allLog.size() - 1; j >= 0; --j) {
            if (!m_canRun) {
                return;
            }
            QString str = allLog.value(j);
            QRegularExpressionMatch match = re.match(str);
            bool matchRes = match.hasMatch();
            if (matchRes) {
                //时间搜索条件
                QDateTime dt = QDateTime::fromString(match.captured(1) + match.captured(2), "yyyy-MM-ddhh:mm:ss");
                QDateTime localdt = dt.toLocalTime();
                //日志等级筛选条件
                QString logLevel = match.captured(3);
                //不满足条件的情况下继续搜索
                if (dt.toMSecsSinceEpoch() < m_dnfFilters.timeFilter || (m_dnfFilters.levelfilter != DNFLVALL && m_dnfLevelDict.value(logLevel) != m_dnfFilters.levelfilter))
                    continue;
                //记录日志等级，时间和主体信息
                dnfLog.level = m_transDnfDict.value(logLevel);
                dnfLog.dateTime = localdt.toString("yyyy-MM-dd hh:mm:ss");
                dnfLog.msg = match.captured(4) + multiLine;
                dList.append(dnfLog);
                multiLine.clear();
            } else {
                //如果不匹配，认为是多条信息，添加换行符，在前一条信息后添加信息。
                if (!str.trimmed().isEmpty() && !dList.isEmpty()) {
                    multiLine.push_front("\n" + str);
                }
            }
            if (!m_canRun) {
                return;
            }
        }
    }
    emit dnfFinished(dList);
}

void LogAuthThread::handleDmesg()
{
    QList<LOG_MSG_DMESG> dmesgList;
    if (!m_canRun) {
        return;
    }
    QString startStr = startTime();
    QDateTime curDt = QDateTime::currentDateTime();

    if (startStr.isEmpty()) {
        emit dmesgFinished(dmesgList);
        return;
    }
    if (!m_canRun) {
        return;
    }

    initProccess();
    //共享内存对应变量置true，允许进程内部逻辑运行
    ShareMemoryInfo shareInfo;
    shareInfo.isStart = true;
    SharedMemoryManager::instance()->setRunnableTag(shareInfo);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    m_process->start("pkexec", QStringList() << "logViewerAuth"
                                             << "dmesg" << SharedMemoryManager::instance()->getRunnableKey());
    m_process->waitForFinished(-1);
    QString errorStr(m_process->readAllStandardError());
    Utils::CommandErrorType commandErrorType = Utils::isErroCommand(errorStr);
    if (!m_canRun) {
        return;
    }
    if (commandErrorType != Utils::NoError) {
        if (commandErrorType == Utils::PermissionError) {
            emit proccessError(errorStr + "\n" + "Please use 'sudo' run this application");
        } else if (commandErrorType == Utils::RetryError) {
            emit proccessError("The password is incorrect,please try again");
        }
        m_process->close();
        return;
    }
    if (!m_canRun) {
        return;
    }
    QByteArray outByte = m_process->readAllStandardOutput();
    QByteArray byte = Utils::replaceEmptyByteArray(outByte);
    QTextStream stream(&byte);
    QByteArray encode;
    stream.setCodec(encode);
    stream.readAll();
    QStringList l = QString(byte).split('\n');
    m_process->close();
    if (!m_canRun) {
        return;
    }
    qint64 curDtSecond = curDt.toMSecsSinceEpoch() - static_cast<int>(startStr.toDouble() * 1000);
    for (QString str : l) {
        if (!m_canRun) {
            return;
        }
        str.replace(QRegExp("\\x1B\\[\\d+(;\\d+){0,2}m"), "");
        QRegExp dmesgExp("^\\<([0-7])\\>\\[\\s*[+-]?(0|([1-9]\\d*))(\\.\\d+)?\\](.*)");
        //启用贪婪匹配
        dmesgExp.setMinimal(false);
        int pos = dmesgExp.indexIn(str);
        if (pos >= 0) {
            QStringList list = dmesgExp.capturedTexts();
            if (list.count() < 6)
                continue;
            QString timeStr = list[3] + list[4];
            QString msgInfo = list[5].simplified();
            int levelOrigin = list[1].toInt();
            QString tStr = timeStr.split("[", QString::SkipEmptyParts)[0].trimmed();
            qint64 realT = curDtSecond + qint64(tStr.toDouble() * 1000);
            QDateTime realDt = QDateTime::fromMSecsSinceEpoch(realT);
            if (realDt.toMSecsSinceEpoch() < m_dmesgFilters.timeFilter) // add by Airy
                continue;
            if (m_dmesgFilters.levelFilter != LVALL) {
                if (levelOrigin != m_dmesgFilters.levelFilter)
                    continue;
            }
            LOG_MSG_DMESG msg;
            msg.dateTime = realDt.toString("yyyy-MM-dd hh:mm:ss.zzz");
            msg.msg = msgInfo;
            msg.level = m_levelMap.value(levelOrigin);
            dmesgList.insert(0, msg);
        } else {
            if (dmesgList.length() > 0) {
                dmesgList[0].msg += str;
            }
        }
        if (!m_canRun) {
            return;
        }
    }
    emit dmesgFinished(dmesgList);
}

void LogAuthThread::handleAudit()
{
    QList<LOG_MSG_AUDIT> aList;
    for (int i = 0; i < m_FilePath.count(); i++) {
        if (!m_FilePath.at(i).contains("txt")) {
            if (!DLDBusHandler::instance(this)->isFileExist(m_FilePath.at(i))) {
                emit auditFinished(m_threadCount);
                return;
            }
        }
        if (!m_canRun) {
            return;
        }

        if (!Utils::runInCmd) {
            initProccess();
            if (!m_canRun) {
                return;
            }
            m_process->setProcessChannelMode(QProcess::MergedChannels);
            if (!m_canRun) {
                return;
            }

            if (DBusManager::isSEOpen()) {
                if (DBusManager::isAuditAdmin()) {
                    // 是审计管理员，需要鉴权，有错则传出空数据
                    if (!Utils::checkAuthorization("com.deepin.pkexec.logViewerAuth.self", QCoreApplication::instance()->applicationPid())) {
                        emit auditFinished(m_threadCount);
                        return;
                    }
                } else {
                    // 不是审计管理员，给出提示
                    emit auditFinished(m_threadCount, true);
                    return;
                }
            } else {
                // 未开启等保四，鉴权逻辑同内核日志
                ShareMemoryInfo shareInfo;
                shareInfo.isStart = true;
                SharedMemoryManager::instance()->setRunnableTag(shareInfo);
                //启动日志需要提权获取，运行的时候把对应共享内存的名称传进去，方便获取进程拿标记量判断是否继续运行
                m_process->start("pkexec", QStringList() << "logViewerAuth"
                                 << m_FilePath.at(i) << SharedMemoryManager::instance()->getRunnableKey());
                m_process->waitForFinished(-1);
                if (m_process->exitCode() != 0) {
                    emit auditFinished(m_threadCount);
                    return;
                }
            }
        }

        if (!m_canRun) {
            return;
        }

        QString byte;
        if (Utils::convertToMB(DLDBusHandler::instance(this)->getFileSize(m_FilePath.at(i))) > DBUS_THRESHOLD_MAX) {
            // 日志文件超过100MB，使用文本流读取日志数据，避免DBUS接口被数据流量撑爆
            auto token = DLDBusHandler::instance(this)->openLogStream(m_FilePath.at(i));
            while(1) {
                auto temp = DLDBusHandler::instance(this)->readLogInStream(token);

                if(temp.isEmpty()) {
                    break;
                }

                byte += temp;
            }
        } else {
            byte = DLDBusHandler::instance(this)->readLog(m_FilePath.at(i));
        }

        byte.replace('\u0000', "").replace("\x01", "");
        QStringList strList = byte.split('\n', QString::SkipEmptyParts);

        QRegularExpression re;
        QRegularExpressionMatch match;
        for (int j = strList.size() - 1; j >= 0; --j) {
            if (!m_canRun) {
                return;
            }
            QString str = strList.at(j);
            if (str.isEmpty() || str.indexOf("type=") == -1)
                continue;

            LOG_MSG_AUDIT msg;
            //删除颜色格式字符
            str.replace(QRegExp("\\#033\\[\\d+(;\\d+){0,2}m"), "");
            // remove Useless characters
            str.replace(QRegExp("\\x1B\\[\\d+(;\\d+){0,2}m"), "");
            QStringList list = str.split(" ", QString::SkipEmptyParts);
            if (list.size() < 2)
                continue;

            // 事件类型
            QString eventType = list[0].split("=").last();
            msg.eventType = eventType;

            // 审计类型
            QString auditType = "";
            // 根据事件类型识别审计类型
            if (auditType.isEmpty())
                auditType = Utils::auditType(msg.eventType);

            // 根据事件类型未识别出审计类型
            if (auditType.isEmpty()) {
                // 判断是否为远程连接审计日志
                re.setPattern("(?<=addr=)([^= \"]+|(\"(\\\\\"|[^\"])*\"))");
                match = re.match(str);
                if (match.hasMatch()) {
                    QString addr = match.captured(0);
                    if (QRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b").exactMatch(addr))
                        auditType = Audit_Remote;
                }

                if (auditType.isEmpty()) {
                     // 获取key值，识别出一些特殊的审计类型(主要为自定义的审计类型)
                    re.setPattern("(?<=key=)([^= \"]+|(\"(\\\\\"|[^\"])*\"))");
                    match = re.match(str);
                    if (match.hasMatch()) {
                        QString key = match.captured(0);
                        key.replace("\"","");
                        if (!key.isEmpty())
                            auditType = Utils::auditType(key);
                    }
                }
            }

            // 审计类型依然为空，归为其他类型
            if (auditType.isEmpty())
                auditType = Audit_Other;

            msg.auditType = auditType;

            // 时间"(?<=msg=audit\()[^\.]*(?=\.)"
            re.setPattern("(?<=msg=audit\\()[^\\.]*(?=\\.)");
            match = re.match(str);
            if (match.hasMatch()) {
                QDateTime dateTime = QDateTime::fromTime_t(match.captured(0).toUInt());
                qint64 iTime = dateTime.toMSecsSinceEpoch();
                //对时间筛选
                if (m_auditFilters.timeFilterBegin > 0 && m_auditFilters.timeFilterEnd > 0) {
                    if (iTime < m_auditFilters.timeFilterBegin || iTime > m_auditFilters.timeFilterEnd)
                        continue;
                }
                msg.dateTime = dateTime.toString("yyyy-MM-dd hh:mm:ss");
            }

            // 进程名
            QString processName = "";
            re.setPattern("(?<=comm=\\\")([^= \"]+|(\"(\\\\\"|[^\"])*\"))");
            match = re.match(str);
            if (match.hasMatch()) {
                processName = match.captured(0);
                processName.replace("\"","");
            }

            if (processName.isEmpty()) {
                re.setPattern("(?<=exe=\\\")([^= \"]+|(\"(\\\\\"|[^\"])*\"))");
                match = re.match(str);
                if (match.hasMatch()) {
                    processName = match.captured(0);
                    processName.replace("\"","");
                    processName = processName.split("/").last();
                }
            }

            if (processName.isEmpty())
                processName = "N/A";
            msg.processName = processName;

            // 状态
            QString status = "";
            re.setPattern("(?<=success=)([^= \"]+|(\"(\\\\\"|[^\"])*\"))");
            match = re.match(str);
            if (match.hasMatch()) {
                status = match.captured(0);
                if (status == "yes")
                    status = "OK";
                else
                    status = "Failed";
            }

            if (status.isEmpty()) {
                re.setPattern("(?<=res=)([^= ']+|('(\\\\'|[^'])*'))");
                match = re.match(str);
                if (match.hasMatch()) {
                    status = match.captured(0);
                    if (status == "success")
                        status = "OK";
                    else
                        status = "Failed";
                }

                if (status.isEmpty())
                    status = "OK";
            }

            msg.status = status;

            // 信息，将“msg=audit(1688526389.214:61):”之后的内容作为详细信息
            msg.msg = str.right(str.length() - str.indexOf("):") - 3);

            // 原文
            msg.origin = str;

            aList.append(msg);
            if (!m_canRun) {
                return;
            }
            //每获得500个数据就发出信号给控件加载
            if (aList.count() % SINGLE_READ_CNT == 0) {
                emit auditData(m_threadCount, aList);
                aList.clear();
            }
            if (!m_canRun) {
                return;
            }
        }
    }
    //最后可能有余下不足500的数据
    if (aList.count() >= 0) {
        emit auditData(m_threadCount, aList);
    }
    emit auditFinished(m_threadCount);
}

void LogAuthThread::handleCoredump()
{
    if (!m_canRun) {
        return;
    }
    QList<LOG_MSG_COREDUMP> coredumpList;

    QString byte;
    initProccess();
    if (Utils::runInCmd) {
        byte = DLDBusHandler::instance()->readLog("coredump");
        byte = byte.replace('\u0000', "").replace("\x01", "");
    } else {
        m_process->start("pkexec", QStringList() << "logViewerAuth" <<
                         QStringList() << "coredumpctl-list" << SharedMemoryManager::instance()->getRunnableKey());
        m_process->waitForFinished(-1);
        QByteArray outByte = m_process->readAllStandardOutput();
        byte = Utils::replaceEmptyByteArray(outByte);
    }

    QStringList strList =  QString(byte).split('\n', QString::SkipEmptyParts);

    QRegExp re("(Storage: )\\S+");
    for (int i = strList.size() - 1; i >= 0 ; --i)  {
        QString str = strList.at(i);
        if (!m_canRun) {
            return;
        }
        if (str.trimmed().isEmpty()) {
            continue;
        }
        QStringList tmpList = str.split(" ", QString::SkipEmptyParts);
        if (tmpList.count() < 10)
            continue;

        LOG_MSG_COREDUMP coredumpMsg;
        coredumpMsg.dateTime = tmpList[1] + " " + tmpList[2];
        QDateTime dt = QDateTime::fromString(coredumpMsg.dateTime, "yyyy-MM-dd hh:mm:ss");
        if (m_coredumpFilters.timeFilterBegin > 0 && m_coredumpFilters.timeFilterEnd > 0) {
            if (dt.toMSecsSinceEpoch() < m_coredumpFilters.timeFilterBegin
                    || dt.toMSecsSinceEpoch() > m_coredumpFilters.timeFilterEnd)
                continue;
        }

        // 获取信号名称
        bool isOk = false;
        int sigId = tmpList[7].toInt(&isOk);
        if (isOk && sigId >= 1 && sigId <= sigList.size()) {
            coredumpMsg.sig = sigList[sigId - 1];
        } else {
            coredumpMsg.sig = tmpList[7];
        }
        //获取用户名
        coredumpMsg.uid = tmpList[5];
        coredumpMsg.userName = Utils::getUserNamebyUID(tmpList[5].toUInt());
        coredumpMsg.coreFile = tmpList[8];
        QString exePath = tmpList[9];
        coredumpMsg.exe = exePath;
        coredumpMsg.pid = tmpList[4];

        // 解析coredump文件保存位置
        if (coredumpMsg.coreFile != "missing") {
            // 若coreFile状态为missing，表示文件已丢失，不继续解析文件位置
            QString outInfoByte;
            outInfoByte = DLDBusHandler::instance()->readLog(QString("coredumpctl info %1").arg(coredumpMsg.pid));

            // 解析第一条堆栈信息
            QStringList strList = outInfoByte.split("Stack trace of thread");
            if (strList.size() > 1) {
                coredumpMsg.stackInfo = "Stack trace of thread" + strList[1];
            }
            re.indexIn(outInfoByte);
            coredumpMsg.storagePath = re.cap(0).replace("Storage: ", "");
        } else {
            coredumpMsg.storagePath = QString("coredump file is missing");
        }

        coredumpList.append(coredumpMsg);
        //每获得600个数据就发出信号给控件加载
        if (coredumpList.count() % SINGLE_READ_CNT_COREDUMP == 0) {
            emit coredumpData(m_threadCount, coredumpList);
            coredumpList.clear();
        }
    }

    if (!m_canRun) {
        return;
    }

    //最后可能有余下不足600的数据
    if (coredumpList.count() >= 0) {
        emit coredumpData(m_threadCount, coredumpList);
    }
    emit coredumpFinished(m_threadCount);
}

QString LogAuthThread::readAppLogFromLastLines(const QString& filePath, const int& count)
{
    if (!QFile::exists(filePath)) {
        qCWarning(logAuthWork) << QString("log not existed. path:%1").arg(filePath);
        return "";
    }

    QStringList lines;
    string line;

    ifstream f(filePath.toStdString().c_str(), ios::ate);
    if (f.is_open()) {
        char c;
        streampos size = f.tellg();
        for (int var = 1; var <= size; var++) {
            f.seekg(-var, ios::end);
            f.get(c);

            if (lines.size() >= count)
                break;

            if (c == 0) {
                continue;
            }

            if (c == '\n') {
                if (line.size() > 0) {
                    lines.prepend(line.c_str());
                }
                line = "";
            } else {
                line = c + line;
            }

            c = 0;
        }

        if (line.size() > 0 && lines.size() < count) {
            lines.prepend(line.c_str());
        }

        f.close();
    }

    return lines.join('\n');
}

/**
 * @brief LogAuthThread::initProccess 初始化QProcess指针
 */
void LogAuthThread::initProccess()
{
    if (!m_process) {
        m_process.reset(new QProcess);
    }
}

/**
 * @brief LogAuthThread::formatDateTime 内核日志没有年份 格式为Sep 29 15:53:34 所以需要特殊转换
 * @param m 月份字符串
 * @param d 日期字符串
 * @param t 时间字符串
 * @return 时间毫秒数
 */
qint64 LogAuthThread::formatDateTime(QString m, QString d, QString t)
{
    QLocale local(QLocale::English, QLocale::UnitedStates);

    QDate curdt = QDate::currentDate();

    QString tStr = QString("%1 %2 %3 %4").arg(m).arg(d).arg(curdt.year()).arg(t);
    QDateTime dt = local.toDateTime(tStr, "MMM d yyyy hh:mm:ss");
    return dt.toMSecsSinceEpoch();
}

/**
 * @brief LogAuthThread::formatDateTime 内核日志有年份 格式为2020-01-05 所以需要特殊转换
 * @param y 年月日
 * @param t 时间字符串
 * @return 时间毫秒数
 */
qint64 LogAuthThread::formatDateTime(QString y, QString t)
{
    //when /var/kern.log have the year
    QLocale local(QLocale::English, QLocale::UnitedStates);
    QString tStr = QString("%1 %2").arg(y).arg(t);
    QDateTime dt = local.toDateTime(tStr, "yyyy-MM-dd hh:mm:ss");
    return dt.toMSecsSinceEpoch();
}
