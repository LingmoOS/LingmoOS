/*
    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    SPDX-License-Identifier: GPL-2.0-only

    stubprocess.cpp: Conversation with kdesu_stub.
*/

#include "stubprocess.h"
#include "kcookie_p.h"
#include "stubprocess_p.h"

#include <config-kdesu.h>
#include <ksu_debug.h>

#include <unistd.h>

extern int kdesuDebugArea();

namespace KDESu
{
using namespace KDESuPrivate;

StubProcess::StubProcess()
    : StubProcess(*new StubProcessPrivate)
{
}

StubProcess::StubProcess(StubProcessPrivate &dd)
    : PtyProcess(dd)
{
    m_user = "root";
    m_scheduler = SchedNormal;
    m_priority = 50;
    m_cookie = new KCookie;
    m_XOnly = true;
}

StubProcess::~StubProcess()
{
    delete m_cookie;
}

void StubProcess::setCommand(const QByteArray &command)
{
    m_command = command;
}

void StubProcess::setUser(const QByteArray &user)
{
    m_user = user;
}

void StubProcess::setXOnly(bool xonly)
{
    m_XOnly = xonly;
}

void StubProcess::setPriority(int prio)
{
    if (prio > 100) {
        m_priority = 100;
    } else if (prio < 0) {
        m_priority = 0;
    } else {
        m_priority = prio;
    }
}

void StubProcess::setScheduler(int sched)
{
    m_scheduler = sched;
}

void StubProcess::writeString(const QByteArray &str)
{
    QByteArray out;
    out.reserve(str.size() + 8);
    for (const uchar c : str) {
        if (c < 32) {
            out.append('\\');
            out.append(c + '@');
        } else if (c == '\\') {
            out.append('\\');
            out.append('/');
        } else {
            out.append(c);
        }
    }
    writeLine(out);
}

/*
 * Map pid_t to a signed integer type that makes sense for QByteArray;
 * only the most common sizes 16 bit and 32 bit are special-cased.
 */
template<int T>
struct PIDType {
    typedef pid_t PID_t;
};
template<>
struct PIDType<2> {
    typedef qint16 PID_t;
};
template<>
struct PIDType<4> {
    typedef qint32 PID_t;
};

/*
 * Conversation with kdesu_stub. This is how we pass the authentication
 * tokens (X11) and other stuff to kdesu_stub.
 * return values: -1 = error, 0 = ok, 1 = kill me
 */

int StubProcess::converseStub(int check)
{
    QByteArray line;
    QByteArray tmp;

    while (1) {
        line = readLine();
        if (line.isNull()) {
            return -1;
        }

        if (line == "kdesu_stub") {
            // This makes parsing a lot easier.
            enableLocalEcho(false);
            if (check) {
                writeLine("stop");
            } else {
                writeLine("ok");
            }
            break;
        }
    }

    while (1) {
        line = readLine();
        if (line.isNull()) {
            return -1;
        }

        if (line == "display") {
            writeLine(display());
        } else if (line == "display_auth") {
#if HAVE_X11
            writeLine(displayAuth());
#else
            writeLine("");
#endif
        } else if (line == "command") {
            writeString(m_command);
        } else if (line == "path") {
            QByteArray path = qgetenv("PATH");
            if (!path.isEmpty() && path[0] == ':') {
                path = path.mid(1);
            }
            if (m_user == "root") {
                if (!path.isEmpty()) {
                    path = "/sbin:/bin:/usr/sbin:/usr/bin:" + path;
                } else {
                    path = "/sbin:/bin:/usr/sbin:/usr/bin";
                }
            }
            writeLine(path);
        } else if (line == "user") {
            writeLine(m_user);
        } else if (line == "priority") {
            tmp.setNum(m_priority);
            writeLine(tmp);
        } else if (line == "scheduler") {
            if (m_scheduler == SchedRealtime) {
                writeLine("realtime");
            } else {
                writeLine("normal");
            }
        } else if (line == "xwindows_only") {
            if (m_XOnly) {
                writeLine("no");
            } else {
                writeLine("yes");
            }
        } else if (line == "app_startup_id") {
            const QList<QByteArray> env = environment();
            QByteArray tmp;
            static const char startup_env[] = "DESKTOP_STARTUP_ID=";
            static const std::size_t size = sizeof(startup_env);
            for (const auto &var : env) {
                if (var.startsWith(startup_env)) {
                    tmp = var.mid(size - 1);
                }
            }
            if (tmp.isEmpty()) {
                tmp = "0"; // krazy:exclude=doublequote_chars
            }
            writeLine(tmp);
        } else if (line == "app_start_pid") { // obsolete
            // Force the pid_t returned from getpid() into
            // something QByteArray understands; avoids ambiguity
            // between short and unsigned short in particular.
            tmp.setNum((PIDType<sizeof(pid_t)>::PID_t)(getpid()));
            writeLine(tmp);
        } else if (line == "environment") { // additional env vars
            const QList<QByteArray> env = environment();
            for (const auto &var : env) {
                writeString(var);
            }
            writeLine("");
        } else if (line == "end") {
            return 0;
        } else {
            qCWarning(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                               << "Unknown request:" << line;
            return 1;
        }
    }

    return 0;
}

QByteArray StubProcess::display()
{
    return m_cookie->display();
}

QByteArray StubProcess::displayAuth()
{
#if HAVE_X11
    return m_cookie->displayAuth();
#else
    return QByteArray();
#endif
}

void StubProcess::virtual_hook(int id, void *data)
{
    PtyProcess::virtual_hook(id, data);
}

} // namespace KDESu
