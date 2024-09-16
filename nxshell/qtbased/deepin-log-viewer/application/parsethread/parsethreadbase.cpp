// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "parsethreadbase.h"

#include <QDebug>
#include <QDateTime>
using namespace std;

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(logParseWork, "org.deepin.log.viewer.parse.work")
#else
Q_LOGGING_CATEGORY(logParseWork, "org.deepin.log.viewer.parse.work", QtInfoMsg)
#endif

int ParseThreadBase::thread_count = 0;

/**
 * @brief ParseThreadBase::ParseThreadBase 构造函数
 * @param parent 父对象
 */
ParseThreadBase::ParseThreadBase(QObject *parent)
    :  QObject(parent)
    , QRunnable()
    , m_type(NONE)
{
    qRegisterMetaType<ParseThreadBase::Status>("ParseThreadBase::Status");
    //使用线程池启动该线程，跑完自己删自己
    setAutoDelete(true);
    //静态计数变量加一并赋值给本对象的成员变量，以供外部判断是否为最新线程发出的数据信号
    thread_count++;
    m_threadCount = thread_count;

    initConnect();
}

/**
 * @brief ParseThreadBase::~ParseThreadBase 析构函数，停止并销毁process指针
 */
ParseThreadBase::~ParseThreadBase()
{
    stopProccess();
}

void ParseThreadBase::initConnect()
{
    m_pParser = dynamic_cast<LogFileParser*>(this->parent());
    if (m_pParser) {
        connect(this, &ParseThreadBase::parseFinished, m_pParser, &LogFileParser::parseFinished);
        connect(this, &ParseThreadBase::logData, m_pParser, &LogFileParser::logData);
        connect(m_pParser, &LogFileParser::stop, this, &ParseThreadBase::stopProccess);
    }
}

/**
 * @brief ParseThreadBase::initProccess 初始化QProcess指针
 */
void ParseThreadBase::initProccess()
{
    if (!m_process) {
        m_process.reset(new QProcess);
    }
}

/**
 * @brief ParseThreadBase::stopProccess 停止日志数据获取进程并销毁
 */
void ParseThreadBase::stopProccess()
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

void ParseThreadBase::setFilter(LOG_FILTER_BASE &filter)
{
    m_type = filter.type;
    m_filter = filter;
}

int ParseThreadBase::getIndex()
{
    return  m_threadCount;
}

/**
 * @brief ParseThreadBase::run 线程执行虚函数
 */
void ParseThreadBase::run()
{
    m_canRun = false;
}
