// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "process_db.h"

#include "wm/wm_window_list.h"
#include "process/desktop_entry_cache.h"
#include "process/process_icon.h"
#include "process/process_icon_cache.h"
#include "process/process_name.h"
#include "process/process_name_cache.h"
#include "process/process_controller.h"

#include <QReadLocker>
#include <QWriteLocker>
#include <QApplication>
#include <QDebug>

#include <sys/resource.h>

using namespace core::wm;

namespace core {
namespace process {

const int DesktopEntryTimeCount = 150; // 5 minutes interval
ProcessDB::ProcessDB(QObject *parent)
    : QObject(parent)
{
    m_procSet = new ProcessSet();
    m_windowList = new WMWindowList();
    m_desktopEntryCache = new DesktopEntryCache();

    m_desktopEntryTimeCount = DesktopEntryTimeCount;

    m_euid = geteuid();
}

ProcessDB::~ProcessDB()
{
    if (m_procSet) {
        delete m_procSet;
        m_procSet = nullptr;
    }
    if (m_windowList) {
        delete m_windowList;
        m_windowList = nullptr;
    }
    if (m_desktopEntryCache) {
        delete m_desktopEntryCache;
        m_desktopEntryCache = nullptr;
    }
}

ProcessDB *ProcessDB::instance()
{
    auto *thread = ThreadManager::instance()->thread<SystemMonitorThread>(BaseThread::kSystemMonitorThread);
    return thread->systemMonitorInstance()->processDB();
}

uid_t ProcessDB::processEuid()
{
    return m_euid;
}

ProcessSet *ProcessDB::processSet()
{
    return m_procSet;
}

WMWindowList *ProcessDB::windowList()
{
    return m_windowList;
}

DesktopEntryCache *ProcessDB::desktopEntryCache()
{
    return m_desktopEntryCache;
}

bool ProcessDB::isCurrentProcess(pid_t pid)
{
    pid_t cur = getpid();
    return pid == cur;
}

void ProcessDB::endProcess(pid_t pid)
{
    sendSignalToProcess(pid, SIGTERM);
}

void ProcessDB::pauseProcess(pid_t pid)
{
    sendSignalToProcess(pid, SIGSTOP);
}

void ProcessDB::resumeProcess(pid_t pid)
{
    sendSignalToProcess(pid, SIGCONT);
}

void ProcessDB::killProcess(pid_t pid)
{
    sendSignalToProcess(pid, SIGKILL);
}

void ProcessDB::update()
{
    if (m_desktopEntryTimeCount++ && m_desktopEntryTimeCount >= DesktopEntryTimeCount) {
        m_desktopEntryTimeCount = 0;
        m_desktopEntryCache->updateCache();
    }

    m_windowList->updateWindowListCache();
    m_procSet->refresh();
}

void ProcessDB::setProcessPriority(pid_t pid, int priority)
{
    emit signalProcessPrioritysetChanged(pid, priority);
}

void ProcessDB::sendSignalToProcess(pid_t pid, int signal)
{
    ErrorContext ec = {};
    auto errfmt = [ = ](decltype(errno) err, const QString & title, int p, int sig,
    ErrorContext & ectx) -> ErrorContext & {
        ectx.setCode(ErrorContext::kErrorTypeSystem);
        ectx.setSubCode(err);
        ectx.setErrorName(title);
        QString errmsg = QString("PID: %1, Signal: [%2], Error: [%3] %4")
        .arg(p)
        .arg(sig)
        .arg(err)
        .arg(strerror(err));
        ectx.setErrorMessage(errmsg);
        return ectx;
    };
    auto emitSignal = [ = ](int signal) {
        if (signal == SIGTERM) {
            Q_EMIT processEnded(pid);
        } else if (signal == SIGSTOP) {
            Q_EMIT processPaused(pid, 'T');
        } else if (signal == SIGCONT) {
            Q_EMIT processResumed(pid, 'R');
        } else if (signal == SIGKILL) {
            Q_EMIT processKilled(pid);
        } else {
            qCWarning(app) << "Unexpected signal in this case:" << signal;
        }
    };
    auto fmsg = [ = ](int signal) -> QString {
        if (signal == SIGTERM)
        {
            return QApplication::translate("Process.Signal", "Failed to end process");
        } else if (signal == SIGSTOP)
        {
            return QApplication::translate("Process.Signal", "Failed to pause process");
        } else if (signal == SIGCONT)
        {
            return QApplication::translate("Process.Signal", "Failed to resume process");
        } else if (signal == SIGKILL)
        {
            return QApplication::translate("Process.Signal", "Failed to kill process");
        } else
        {
            return QApplication::translate("Process.Signal", "Unknown error");
        }
    };
    auto pctl = [ = ](pid_t pid, int signal) {
        // call pkexec to promote
        auto *ctrl = new ProcessController(pid, signal, this);
        connect(ctrl, &ProcessController::resultReady, this,
        [this, errfmt, emitSignal, pid, signal, fmsg](int code) {
            if (code != 0) {
                ErrorContext ec1 = {};
                ec1 = errfmt(code, fmsg(signal), pid, signal, ec1);
                Q_EMIT processControlResultReady(ec1);
            } else {
                emitSignal(signal);
            }
        });
        connect(ctrl, &ProcessController::finished, ctrl, &QObject::deleteLater);
        ctrl->execute();
    };
    auto pkill = [this, pctl, errfmt, emitSignal, fmsg](pid_t pid, int signal) {
        int rc = 0;
        errno = 0;
        ErrorContext ec1 = {};
        rc = kill(pid, signal);
        if (rc == -1 && errno != 0) {
            if (errno == EPERM) {
                pctl(pid, signal);
            } else {
                ec1 = errfmt(errno, fmsg(signal), pid, signal, ec1);
                Q_EMIT processControlResultReady(ec1);
                return;
            }
        } else {
            emitSignal(signal);
        }
    };

    if (signal == SIGTERM || signal == SIGKILL) {
        ec = {};
        errno = 0;
        int rc = 0;
        // send SIGCONT first, otherwise signal will hang
        rc = kill(pid, SIGCONT);
        if (rc == -1 && errno != 0) {
            // not authorized, use pkexec instead
            if (errno == EPERM) {
                pctl(pid, signal);
            } else {
                ec = errfmt(
                         errno,
                         QApplication::translate("Process.Signal", "Failed in sending signal to process"),
                         pid, SIGCONT, ec);
                Q_EMIT processControlResultReady(ec);
                return;
            }
        } else {
            pkill(pid, signal);
        }
    } else {
        pkill(pid, signal);
    }
}

} // namespace process
} // namespace core
