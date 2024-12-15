// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginprocess.h"
#include "utils/specialtools.h"

#include <QProcess>
#include <QTimerEvent>

using namespace GrandSearch;

#define MAXRESTARTCOUNT 3

PluginProcess::PluginProcess(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QProcess::ProcessState>();
}

PluginProcess::~PluginProcess()
{
    clear();
}

bool PluginProcess::addProgram(const QString &name, const QString &path)
{
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT(!path.isEmpty());

    //解析出可执行程序与参数
    QString exec;
    QStringList args;
    {
        if (!SpecialTools::splitCommand(path, exec, args)) {
            qWarning() << "error cmd:" << path;
            return false;
        }
    }

    if (m_programs.contains(name))
        return false;

    m_programs.insert(name, path);

    //创建进程
    QProcess *process = new QProcess;
    process->setProgram(exec);
    process->setArguments(args);
    m_processes.insert(name, process);

    //进程状态改变,必须使用队列，否则在槽函数中继续启动进程会导致无法发出进程状态的信号
    connect(process, &QProcess::stateChanged, this, &PluginProcess::processStateChanged, Qt::QueuedConnection);
    return true;
}

void PluginProcess::setWatched(const QString &name, bool watch)
{
    if (watch) {
        m_watch.insert(name, watch);
    } else {
        m_watch.remove(name);
    }
}

void PluginProcess::clear()
{
    m_programs.clear();
    m_watch.clear();
    m_restartCount.clear();
    auto processes = std::move(m_processes);

    for (QProcess *process : processes.values()) {
        process->terminate();
        delete process;
    }

    auto checklist = std::move(m_checklist);
    for (int id : checklist.values()) {
        killTimer(id);
    }
}

void PluginProcess::timerEvent(QTimerEvent *event)
{
    int id = event->timerId();
    if (auto process = m_checklist.key(id, nullptr)) {
        qDebug() << "check" << id << process->program();
        //检查
        checkStability(process);

        //只检查一次，移除timer
        removeChecklist(process);
        return;
    }
    QObject::timerEvent(event);
}

bool PluginProcess::startProgram(const QString &name)
{
    if (!m_processes.contains(name)) {
        qWarning() << "no such program:" << name;
        return false;
    }

    auto process = m_processes.value(name);
    Q_ASSERT(process);

    QMap<QProcess*, int>::iterator iter = m_restartCount.find(process);
    if (iter != m_restartCount.end()) {
        iter.value() += 1;
        if (iter.value() > MAXRESTARTCOUNT) {
            if (iter.value() == (MAXRESTARTCOUNT + 1))
                qWarning() << "reject to start program:" << name << process->program() << "too many failures" << iter.value();
            return false;
        }
    } else {
        m_restartCount.insert(process, 1);
    }

    process->terminate();
    process->start();
    qInfo() << "start program:" << name << process->program() << process->arguments();
    return true;
}

void PluginProcess::terminate(const QString &name)
{
    auto iter = m_processes.find(name);
    if (iter != m_processes.end()) {
        QProcess *process = iter.value();
        Q_ASSERT(process);
        if (process->state() == QProcess::NotRunning)
            return;

        //取消状态监控，防止被重启
        disconnect(process, nullptr, this, nullptr);

        qInfo() << "terminate" << name;
        process->terminate();

        //等待1s退出
        if(!process->waitForFinished(1000)) {
            //没有退出，直接kill
            process->kill();
            qWarning() << "kill proccess" << name;
        }
        qInfo() << name << "is terminated";

        //清理启动状态
        removeChecklist(process);
        m_restartCount.remove(process);

        //再次连接，进程状态改变,必须使用队列，否则在槽函数中继续启动进程会导致无法发出进程状态的信号
        connect(process, &QProcess::stateChanged, this, &PluginProcess::processStateChanged, Qt::QueuedConnection);
    }
}

void PluginProcess::processStateChanged()
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process)
        return;

    auto state = process->state();
    qInfo() << "process state:" << m_processes.key(process) << state << process->pid();
    if (state == QProcess::Running) {
        //校验启动成功的时间
        addChecklist(process);
        return;
    }

    if (state == QProcess::NotRunning) {
        auto name = m_processes.key(process);
        //守护程序，自动启动
        if (m_watch.contains(name))
            startProgram(name);
    }
}

void PluginProcess::removeChecklist(QProcess *process)
{
    auto iter = m_checklist.find(process);
    if (iter != m_checklist.end()) {
        //停止定时器
        if (iter.value() > 0)
            killTimer(iter.value());

        m_checklist.remove(process);
    }
}

void PluginProcess::addChecklist(QProcess *process)
{
    Q_ASSERT(process);

    //先移除旧的
    removeChecklist(process);

    //1min后检查进程是否已稳定运行
    int id = startTimer(60 * 1000);
    if (id > 0) {
        m_checklist.insert(process, id);
    } else {
       qCritical() << "fialed to start timer :" << id << m_processes.key(process);
    }
}

void PluginProcess::checkStability(QProcess *process)
{
    Q_ASSERT(process);
    if (process->state() == QProcess::Running) {
        qInfo() << "process" << m_processes.key(process)
                << "is stable. pid:" << process->pid()
                << m_restartCount.value(process);
        m_restartCount.remove(process);
    } else {
        qWarning() << "process" << m_processes.key(process)
                   << "is unstable." << process->program()
                   << m_restartCount.value(process);
    }
}
