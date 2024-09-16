// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "getpiddetailname.h"
#include "csingleteon.h"
#include "../common/process_tree.h"
#include "../common/common.h"
//#include "defenderprocessinfo.h"

#include <DSimpleListItem>

#include <QProcess>
#include <QDir>
#include <QMap>
#include <QDebug>
#include <QThread>
#include <QTimer>

const QString service = "com.deepin.defender.analysisapp";
const QString path = "/com/deepin/defender/analysisapp";
const QString defaultWineName = "wineserver.real";
const int updateIntervel = 2;
const int abandonIntervel = 100;

GetPidDetailName::GetPidDetailName()
{
    //注册自定义类型
    qRegisterMetaType<QList<QPair<int, processStruct>>>("QList<QPair<int, processStruct>>");

//    QThread *thread = new QThread;
//    moveToThread(thread);
//    thread->start();

    connect(this, &GetPidDetailName::reqStartTimer, this, &GetPidDetailName::onReqStartTimer);
    Q_EMIT reqStartTimer();
}

GetPidDetailName::~GetPidDetailName()
{

}

//获取当前目录全部文件(不包括文件夹)
void GetPidDetailName::GetFileList(QString dir)
{
    m_procList.clear();
    QDir dirPath(dir);
    QList<QFileInfo> file(dirPath.entryInfoList());
    QString name = "";
    int processaId = -1;

    for (auto it = file.begin(); it != file.end(); it++) {
        name = it->fileName();
        processaId = name.toInt();

        //(*it).isFile()可以获取全部文件名称，(*it).isDir()获取全部文件夹名称
        if ((*it).isDir() && processaId) {
            m_procList.push_back(processaId);
        }
    }

    Q_EMIT notifyUpdateGuiProcList();
}

processStruct GetPidDetailName::getDesktopName(const QString &sPid)
{
    processStruct ret;
    ret.title = "";
    ret.desktop = "";

    // 取进程执行命令行路径
    QFile file(QString("/proc/%1/exe").arg(sPid));
    QString sCmdLinePath;
    if (file.open(QFile::ReadOnly)) {
        sCmdLinePath = file.readLink();
        sCmdLinePath = sCmdLinePath.split(" ").at(0);
        file.close();
    }
#if log_switch
    qDebug() << "[wubw]: sCmdLinePath: " << sCmdLinePath ;
#endif

    // 判断第三方应用
    QStringList list = sCmdLinePath.split("/");
    if (list.at(list.count() - 1) == defaultWineName) {
        QString sDeskTop = getProcessEnvironmentVariable(sPid.toInt());
#if log_switch
        qDebug() << "[wubw]: sProcL: " << sDeskTop;
#endif
        ret.title = defaultWineName;
        ret.desktop = sDeskTop;
        return ret;
    }

    // 查找包名
    QProcess *processS = new QProcess(this);
    processS->start("dpkg", QStringList() << "-S" << sCmdLinePath);
    processS->waitForStarted();
    processS->waitForFinished();
    QString sProcessResultS = QString::fromLocal8Bit(processS->readAllStandardOutput()).trimmed();
    processS->deleteLater();
#if log_switch
    qDebug() << "[wubw]: sProcS: " << sProcessResultS;
#endif
    QStringList sListS = sProcessResultS.split(":");
    if (sListS.count() > 0) {
#if log_switch
        qDebug() << "[wubw]: sProcS name: " << sListS.at(0);
#endif
        ret.title = sListS.at(0);

        QProcess *processL = new QProcess(this);
        processL->start("dpkg", QStringList() << "-L" << sListS.at(0));
        processL->waitForStarted();
        processL->waitForFinished();
        QString sProcessResultL = QString::fromLocal8Bit(processL->readAllStandardOutput()).trimmed();
        processL->deleteLater();
#if log_switch
        qDebug() << "[wubw]: sProcS: " << sProcessResultL;
#endif
        QStringList sList = sProcessResultL.split("\n");
        for (int i = 0; i < sList.count(); ++i) {
            if (sList.at(i).contains("desktop")) {
#if log_switch
                qDebug() << "[wubw]: sProcL: " << sList.at(i);
#endif
                //截取desktop
//                QStringList sListProcL = sList.at(i).split("/");
//                ret.desktop = sListProcL.at(sListProcL.count() - 1);
                ret.desktop = sList.at(i);
#if log_switch
                qDebug() << "[wubw]: sProcLList: " << ret.desktop;
#endif
            }
        }
    }

    return ret;
}

DefenderProcessInfoList GetPidDetailName::getProcInfoList()
{
    return m_processInfoList;
}

DefenderProcessList GetPidDetailName::getProcList()
{
    return m_procList;
}

QList<QString> GetPidDetailName::getGuiProcDesktopList()
{
    return m_guiDesktioList;
}

QList<QString> GetPidDetailName::getSingleProcDesktopList()
{
    return m_singleProcDesktopList;
}

QString GetPidDetailName::getProcessEnvironmentVariable(int pid, QString environmentName)
{
    return Utils::getProcessEnvironmentVariable(pid, environmentName);
}

QString GetPidDetailName::getProcCmdline(int pid)
{
    QString name = "";
    QFile file(QString("/proc/%1/cmdline").arg(pid));
    if (file.open(QFile::ReadOnly)) {
        QString firstline = file.readLine();
        name = QString(firstline).split("/").last().trimmed();
        file.close();
#if log_switch
        qDebug() << "name = " << name;
#endif
    }
    return name;
}

void GetPidDetailName::updateStatus()
{
    // Read the list of open processes information.
    PROCTAB *proc = openproc(
                        PROC_FILLMEM |          // memory status: read information from /proc/#pid/statm
                        PROC_FILLSTAT |         // cpu status: read information from /proc/#pid/stat
                        PROC_FILLUSR            // user status: resolve user ids to names via /etc/passwd
                    );
    static proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_t));

    StoredProcType processes;
    while (readproc(proc, &proc_info) != nullptr) {
        processes[proc_info.tid] = proc_info;
    }
    closeproc(proc);

    // Read tray icon process.
    QList<int> trayProcessXids = Utils::getTrayWindows();
    QMap<int, int> trayProcessMap;

    for (u_int32_t xid : trayProcessXids) {
        trayProcessMap[m_findWindowTitle->getWindowPid(xid)] = xid;
    }

    // Fill gui chlid process information when filterType is OnlyGUI.
    m_findWindowTitle->updateWindowInfos();
    ProcessTree processTree;// = new ProcessTree();
    processTree.scanProcesses(processes);
    QList<int> guiPids = m_findWindowTitle->getWindowPids();

    // Tray pid also need add in gui pids list.
    for (int pid : trayProcessMap.keys()) {
        if (!guiPids.contains(pid)) {
            guiPids << pid;
#if log_switch
            qDebug() << "pid = " << pid;
#endif
        }
    }

    //排序
    qSort(guiPids.begin(), guiPids.end());

    int lastPidCount = m_guiPids.count();
    int pidCount = guiPids.count();
    bool bIsPidListSame = true;
    if (lastPidCount != pidCount) {
        bIsPidListSame = false;
    } else {
        for (int i = 0; i < pidCount; i++) {
            if (m_guiPids.at(i) != guiPids.at(i)) {
                bIsPidListSame = false;
                break;
            }
        }
    }

    if (!bIsPidListSame) {
        qDebug() << "＞＞old＞＞ m_guiPids : " << lastPidCount << m_guiPids;
        qDebug() << "＜＜new＜＜   guiPids : " << pidCount << guiPids;

        m_guiPids = guiPids;
    } else {
        //相同的数据直接返回
        return;
    }

    //获取进程的子进程
//    for (int guiPid : guiPids) {
//        QList<int> childPids;
//        childPids = processTree.getAllChildPids(guiPid);
//        qDebug() << " childPids : " << childPids;
//    }

    QList<int> procList;
    QList<QString> guiDesktioList;
    QList<QString> singleProcDesktopList;
    QList<QPair<int, processStruct>> pidNameList;
    processStruct value;
#if dbus_switch
    DefenderProcessInfoList processInfoList;
    DefenderProcessInfo processInfo;
#endif
    for (auto &i : processes) {
        int pid = (&i.second)->tid;

        //只解析gui相关的进程号会导致数据丢失
//        if (!m_guiPids.contains(pid))
//            continue;
        if (m_abandonProcList.contains(pid))
            continue;

        QString cmdline = Utils::getProcessCmdline(pid);
        bool isWineProcess = cmdline.startsWith("c:\\");
        QString name = Utils::getProcessName(&i.second, cmdline);

        std::string utilsDesktopFile =  Utils::getProcessDesktopFile(pid, name, cmdline, trayProcessMap);
        QString desktopFile = QString(utilsDesktopFile.c_str());
        QString title = m_findWindowTitle->getWindowTitle(pid);

        // Record wine application and wineserver.real desktop file.
        // We need transfer wineserver.real network traffic to the corresponding wine program.
        if (name == defaultWineName) {
            // Insert pid<->desktopFile to map to search in all network process list.
            desktopFile = getProcessEnvironmentVariable(pid);
        }

        if (title == "") {
            if (isWineProcess) {
                // If wine process's window title is blank, it's not GUI window process.
                // Title use process name instead.
                title = name;
            } else {
                title = Utils::getDisplayNameFromName(name, utilsDesktopFile);
            }
        }

        if (desktopFile != "") {
#if log_switch
            qDebug() << "[wubw] title, desktopFile : " << pid << title << desktopFile;
#endif

            for (int i = 0; i < m_guiPids.count(); i++) {
                QString name = getProcCmdline(m_guiPids[i]);
                if (isReplacePid(name, desktopFile))
                    m_guiPids[i] = pid;
#if log_switch
                qDebug() << "m_guiPids = " << m_guiPids[i];
#endif
            }
#if dbus_switch
            processInfo.proc = pid;
            processInfo.title = title;
            processInfo.desktop = desktopFile;
            processInfoList.append(processInfo);
#endif
            value.title = title;
            value.desktop = desktopFile;
            pidNameList.append(QPair<int, processStruct>(pid, value));
            procList.append(pid);

            if (m_guiPids.contains(pid)) {
                guiDesktioList.append(desktopFile);

                if (!singleProcDesktopList.contains(desktopFile))
                    singleProcDesktopList.append(desktopFile);
            }
        } else {
            value.title = "";
            value.desktop = "";

            {
                DEFENDER_MUTEX_LOCKER(&SMutex::getInstance().m_mutex);
                m_abandonProcList.append(pid);
            }
        }
    }

//更新pidName列表，先将第一批成功解析的数据发送出去
//    m_pidNameList = pidNameList;
//    m_procList = procList;
//    m_guiDesktioList = guiDesktioList;
//    Q_EMIT notifyUpdateGuiProcList();

//遍历GUI的pid，若procList不包含对应的pid,则说明有一些pid未解析成功
//需要使用getDesktopName再去获取desktop
    for (auto pid : m_guiPids) {
        if (!procList.contains(pid)) {
            processStruct desktopStu = getDesktopName(QString::number(pid));
            if (desktopStu.desktop != "") {
                procList << pid;
                pidNameList << QPair<int, processStruct>(pid, desktopStu);
                //将Gui进程的desktop加入到guiDesktioList
                guiDesktioList.append(desktopStu.desktop);

                if (!singleProcDesktopList.contains(desktopStu.desktop))
                    singleProcDesktopList.append(desktopStu.desktop);
#if dbus_switch
                processInfo.proc = pid;
                processInfo.title = desktopStu.title;
                processInfo.desktop = desktopStu.desktop;
                processInfoList.append(processInfo);
#endif
            }
        }
    }

//再次更新pidName列表，将第二批解析的数据发送出去（这里主要是为了将第一批解析失败的再次解析）
//当前使用解析完后才发送信号，广播出去
    m_procList = procList;
    qSort(m_procList.begin(), m_procList.end());
#if dbus_switch
    m_processInfoList = processInfoList;
    qSort(m_processInfoList.begin(), m_processInfoList.end(), [](const DefenderProcessInfo & a, const DefenderProcessInfo & b) {
        return a.proc < b.proc;
    });
#endif
    m_singleProcDesktopList = singleProcDesktopList;

    m_guiDesktioList = guiDesktioList;
    Q_EMIT notifyUpdateGuiProcList();
#if dbus_switch
    qDebug() << "[wubw] Start GUI Pids : " << m_guiPids.count() << m_guiPids;
    for (auto data : m_processInfoList) {
        //打印desktop不为空的应用，即启动器里面的应用
        if (data.desktop != "")
            qDebug() << "[wubw] Update pid , title, desktop : " << data.proc << data.title << data.desktop;
    }
#endif
    qDebug() << Q_FUNC_INFO << "End";
}

//不能在主线程new对象，给子线程使用
//否则会报警告：QObject: Cannot create children for a parent that is in a different thread.
void GetPidDetailName::onReqStartTimer()
{
    m_findWindowTitle = new FindWindowTitle;
    QTimer *updateTimer = new QTimer;
    connect(updateTimer, &QTimer::timeout, [this] {
        updateStatus();
    });
    updateTimer->start(1000 * updateIntervel);

    //每隔100s清空一次丢弃进程的列表
    QTimer *abandonTimer = new QTimer;
    connect(abandonTimer, &QTimer::timeout, [this] {
        DEFENDER_MUTEX_LOCKER(&SMutex::getInstance().m_mutex);
        m_abandonProcList.clear();
    });
    abandonTimer->start(1000 * abandonIntervel);
}
