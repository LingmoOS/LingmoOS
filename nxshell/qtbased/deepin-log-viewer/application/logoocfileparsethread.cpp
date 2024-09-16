// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logoocfileparsethread.h"
#include "utils.h"
#include "dbusproxy/dldbushandler.h"
#include "sharedmemorymanager.h"

#include <DMessageBox>

#include <QDateTime>
#include <QDebug>
#include <QProcess>
#include <QLoggingCategory>

DWIDGET_USE_NAMESPACE

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logOOCParse, "org.deepin.log.viewer.parse.ooc.work")
#else
Q_LOGGING_CATEGORY(logOOCParse, "org.deepin.log.viewer.parse.ooc.work", QtInfoMsg)
#endif

int LogOOCFileParseThread::thread_count = 0;
/**
 * @brief LogOOCFileParseThread::LogOOCFileParseThread 构造函数
 * @param parent 父对象
 */
LogOOCFileParseThread::LogOOCFileParseThread(QObject *parent)
    : QThread(parent)
{
    //静态计数变量加一并赋值给本对象的成员变量，以供外部判断是否为最新线程发出的数据信号
    thread_count++;
    m_threadCount = thread_count;
}

/**
 * @brief LogOOCFileParseThread::~LogOOCFileParseThread 析构函数，停止并销毁process指针
 */
LogOOCFileParseThread::~LogOOCFileParseThread()
{
    stopProccess();
}

void LogOOCFileParseThread::setParam(const QString &path)
{
    m_path = path;
}

int LogOOCFileParseThread::getIndex()
{
    return m_threadCount;
}

/**
 * @brief LogOOCFileParseThread::doWork 获取数据线程逻辑
 */
void LogOOCFileParseThread::doWork()
{
    //此线程刚开始把可以继续变量置true，不然下面没法跑
    m_canRun = true;

    if (m_path.isEmpty()) {
        emit sigFinished(m_threadCount);
        return;
    }

    if (!checkAuthentication(m_path)) {
        emit sigFinished(m_threadCount, 1);
        return;
    }

    QStringList filePath = DLDBusHandler::instance(this)->getOtherFileInfo(m_path);
    for (int i = 0; i < filePath.count(); i++) {
        if (!m_canRun) {
            emit sigFinished(m_threadCount);
            return;
        }

        //鉴权
        if (!checkAuthentication(filePath.at(i))) {
            emit sigFinished(m_threadCount, 1);
            return;
        }

        m_fileData += DLDBusHandler::instance(this)->readLog(filePath.at(i));
        emit sigData(m_threadCount, m_fileData);
    }

    emit sigFinished(m_threadCount);
}

//鉴权
bool LogOOCFileParseThread::checkAuthentication(const QString &path)
{
    //判断当前用户对文件是否可读
    QFlags <QFileDevice::Permission> power = QFile::permissions(path);
    if (!power.testFlag(QFile::ReadUser)) {
        //若当前用户不具备读取权限，则显示提权对话框
        //共享内存对应变量置true，允许进程内部逻辑运行
        ShareMemoryInfo shareInfo;
        shareInfo.isStart = true;
        SharedMemoryManager::instance()->setRunnableTag(shareInfo);
        //启动日志需要提权获取，运行的时候把对应共享内存的名称传进去，方便获取进程拿标记量判断是否继续运行
        initProccess();
        m_process->start("pkexec", QStringList() << "logViewerAuth"
                         << path << SharedMemoryManager::instance()->getRunnableKey());
        m_process->waitForFinished(-1);
        //有错则传出空数据
        if (m_process->exitCode() != 0) {
            return false;
        }
    }

    return true;
}

//void LogOOCFileParseThread::doWork()
//{
//    //此线程刚开始把可以继续变量置true，不然下面没法跑
//    m_canRun = true;

//    //判断当前用户对文件是否可读
//    QFlags <QFileDevice::Permission> power = QFile::permissions(m_path);
//    if (!power.testFlag(QFile::ReadUser)) {
//        //若当前用户不具备读取权限，则显示提权对话框
//        //共享内存对应变量置true，允许进程内部逻辑运行
//        ShareMemoryInfo shareInfo;
//        shareInfo.isStart = true;
//        SharedMemoryManager::instance()->setRunnableTag(shareInfo);
//        //启动日志需要提权获取，运行的时候把对应共享内存的名称传进去，方便获取进程拿标记量判断是否继续运行
//        initProccess();
//        m_process->start("pkexec", QStringList() << "logViewerAuth"
//                                                 << m_path << SharedMemoryManager::instance()->getRunnableKey());
//        m_process->waitForFinished(-1);
//        //有错则传出空数据
//        if (m_process->exitCode() != 0) {
//            emit sigFinished(m_threadCount, 1);
//            return;
//        }
//    }

//    m_fileData = DLDBusHandler::instance(this)->readLog(m_path);

//    emit sigData(m_threadCount, m_fileData);
//    emit sigFinished(m_threadCount);
//}

/**
 * @brief LogOOCFileParseThread::run 线程执行虚函数重写逻辑
 */
void LogOOCFileParseThread::run()
{
    qCDebug(logOOCParse) << "threadrun";
    doWork();
}

/**
 * @brief LogOOCFileParseThread::stopProccess 停止qprocess获取进程
 */
void LogOOCFileParseThread::stopProccess()
{
    //防止正在执行时重复执行
    if (m_isStopProccess) {
        return;
    }
    m_isStopProccess = true;
    //停止获取线程执行，标记量置false
    m_canRun = false;
    //共享内存数据结构，用于和获取进程共享内存，数据为是否可执行进程，用于控制数据获取进程停止，因为这里会出现需要提权执行的进程，主进程没有权限停止提权进程，所以使用共享内存变量标记量控制提权进程停止
    ShareMemoryInfo   shareInfo ;
    //设置进程为不可执行
    shareInfo.isStart = false;
    //把数据付给共享内存中对应的变量
    SharedMemoryManager::instance()->setRunnableTag(shareInfo);
    if (m_process) {
        m_process->kill();
    }
}

void LogOOCFileParseThread::initProccess()
{
    if (!m_process) {
        m_process.reset(new QProcess);
    }
}
