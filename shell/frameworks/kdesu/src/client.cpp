/*
    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    SPDX-License-Identifier: GPL-2.0-only

    client.cpp: A client for kdesud.
*/

#include "client.h"

#include <config-kdesu.h>
#include <ksu_debug.h>

#include <cerrno>
#include <sys/socket.h>
#include <sys/un.h>

#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <qplatformdefs.h>

extern int kdesuDebugArea();

namespace KDESu
{
class ClientPrivate
{
public:
    ClientPrivate()
        : sockfd(-1)
    {
    }
    QString daemon;
    int sockfd;
    QByteArray sock;
};

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((QT_SOCKLEN_T)(((struct sockaddr_un *)0)->sun_path) + strlen((ptr)->sun_path))
#endif

Client::Client()
    : d(new ClientPrivate)
{
#if HAVE_X11
    QString display = QString::fromLocal8Bit(qgetenv("DISPLAY"));
    if (display.isEmpty()) {
        // we might be on Wayland
        display = QString::fromLocal8Bit(qgetenv("WAYLAND_DISPLAY"));
    }
    if (display.isEmpty()) {
        qCWarning(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                           << "$DISPLAY is not set.";
        return;
    }

    // strip the screen number from the display
    display.remove(QRegularExpression(QStringLiteral("\\.[0-9]+$")));
#else
    QString display = QStringLiteral("NODISPLAY");
#endif

    d->sock = QFile::encodeName(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) + QStringLiteral("/kdesud_") + display);
    connect();
}

Client::~Client()
{
    if (d->sockfd >= 0) {
        close(d->sockfd);
    }
}

int Client::connect()
{
    if (d->sockfd >= 0) {
        close(d->sockfd);
    }
    if (access(d->sock.constData(), R_OK | W_OK)) {
        d->sockfd = -1;
        return -1;
    }

    d->sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (d->sockfd < 0) {
        qCWarning(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                           << "socket():" << strerror(errno);
        return -1;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, d->sock.constData());

    if (QT_SOCKET_CONNECT(d->sockfd, (struct sockaddr *)&addr, SUN_LEN(&addr)) < 0) {
        qCWarning(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                           << "connect():" << strerror(errno);
        close(d->sockfd);
        d->sockfd = -1;
        return -1;
    }

#if !defined(SO_PEERCRED) || !HAVE_STRUCT_UCRED
#if HAVE_GETPEEREID
    uid_t euid;
    gid_t egid;
    // Security: if socket exists, we must own it
    if (getpeereid(d->sockfd, &euid, &egid) == 0 && euid != getuid()) {
        qCWarning(KSU_LOG) << "socket not owned by me! socket uid =" << euid;
        close(d->sockfd);
        d->sockfd = -1;
        return -1;
    }
#else
#ifdef __GNUC__
#warning "Using sloppy security checks"
#endif
    // We check the owner of the socket after we have connected.
    // If the socket was somehow not ours an attacker will be able
    // to delete it after we connect but shouldn't be able to
    // create a socket that is owned by us.
    QT_STATBUF s;
    if (QT_LSTAT(d->sock.constData(), &s) != 0) {
        qCWarning(KSU_LOG) << "stat failed (" << d->sock << ")";
        close(d->sockfd);
        d->sockfd = -1;
        return -1;
    }
    if (s.st_uid != getuid()) {
        qCWarning(KSU_LOG) << "socket not owned by me! socket uid =" << s.st_uid;
        close(d->sockfd);
        d->sockfd = -1;
        return -1;
    }
    if (!S_ISSOCK(s.st_mode)) {
        qCWarning(KSU_LOG) << "socket is not a socket (" << d->sock << ")";
        close(d->sockfd);
        d->sockfd = -1;
        return -1;
    }
#endif
#else
    struct ucred cred;
    QT_SOCKLEN_T siz = sizeof(cred);

    // Security: if socket exists, we must own it
    if (getsockopt(d->sockfd, SOL_SOCKET, SO_PEERCRED, &cred, &siz) == 0 && cred.uid != getuid()) {
        qCWarning(KSU_LOG) << "socket not owned by me! socket uid =" << cred.uid;
        close(d->sockfd);
        d->sockfd = -1;
        return -1;
    }
#endif

    return 0;
}

QByteArray Client::escape(const QByteArray &str)
{
    QByteArray copy;
    copy.reserve(str.size() + 4);
    copy.append('"');
    for (const uchar c : str) {
        if (c < 32) {
            copy.append('\\');
            copy.append('^');
            copy.append(c + '@');
        } else {
            if (c == '\\' || c == '"') {
                copy.append('\\');
            }
            copy.append(c);
        }
    }
    copy.append('"');
    return copy;
}

int Client::command(const QByteArray &cmd, QByteArray *result)
{
    if (d->sockfd < 0) {
        return -1;
    }

    if (send(d->sockfd, cmd.constData(), cmd.length(), 0) != (int)cmd.length()) {
        return -1;
    }

    char buf[1024];
    int nbytes = recv(d->sockfd, buf, 1023, 0);
    if (nbytes <= 0) {
        qCWarning(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                           << "no reply from daemon.";
        return -1;
    }
    buf[nbytes] = '\000';

    QByteArray reply = buf;
    if (reply.left(2) != "OK") {
        return -1;
    }

    if (result) {
        *result = reply.mid(3, reply.length() - 4);
    }
    return 0;
}

int Client::setPass(const char *pass, int timeout)
{
    QByteArray cmd = "PASS ";
    cmd += escape(pass);
    cmd += ' ';
    cmd += QByteArray().setNum(timeout);
    cmd += '\n';
    return command(cmd);
}

int Client::exec(const QByteArray &prog, const QByteArray &user, const QByteArray &options, const QList<QByteArray> &env)
{
    QByteArray cmd;
    cmd = "EXEC ";
    cmd += escape(prog);
    cmd += ' ';
    cmd += escape(user);
    if (!options.isEmpty() || !env.isEmpty()) {
        cmd += ' ';
        cmd += escape(options);
        for (const auto &var : env) {
            cmd += ' ';
            cmd += escape(var);
        }
    }
    cmd += '\n';
    return command(cmd);
}

int Client::setHost(const QByteArray &host)
{
    QByteArray cmd = "HOST ";
    cmd += escape(host);
    cmd += '\n';
    return command(cmd);
}

int Client::setPriority(int prio)
{
    QByteArray cmd;
    cmd += "PRIO ";
    cmd += QByteArray::number(prio);
    cmd += '\n';
    return command(cmd);
}

int Client::setScheduler(int sched)
{
    QByteArray cmd;
    cmd += "SCHD ";
    cmd += QByteArray::number(sched);
    cmd += '\n';
    return command(cmd);
}

int Client::delCommand(const QByteArray &key, const QByteArray &user)
{
    QByteArray cmd = "DEL ";
    cmd += escape(key);
    cmd += ' ';
    cmd += escape(user);
    cmd += '\n';
    return command(cmd);
}
int Client::setVar(const QByteArray &key, const QByteArray &value, int timeout, const QByteArray &group)
{
    QByteArray cmd = "SET ";
    cmd += escape(key);
    cmd += ' ';
    cmd += escape(value);
    cmd += ' ';
    cmd += escape(group);
    cmd += ' ';
    cmd += QByteArray().setNum(timeout);
    cmd += '\n';
    return command(cmd);
}

QByteArray Client::getVar(const QByteArray &key)
{
    QByteArray cmd = "GET ";
    cmd += escape(key);
    cmd += '\n';
    QByteArray reply;
    command(cmd, &reply);
    return reply;
}

QList<QByteArray> Client::getKeys(const QByteArray &group)
{
    QByteArray cmd = "GETK ";
    cmd += escape(group);
    cmd += '\n';
    QByteArray reply;
    command(cmd, &reply);
    int index = 0;
    int pos;
    QList<QByteArray> list;
    if (!reply.isEmpty()) {
        while (1) {
            pos = reply.indexOf('\007', index);
            if (pos == -1) {
                if (index == 0) {
                    list.append(reply);
                } else {
                    list.append(reply.mid(index));
                }
                break;
            } else {
                list.append(reply.mid(index, pos - index));
            }
            index = pos + 1;
        }
    }
    return list;
}

bool Client::findGroup(const QByteArray &group)
{
    QByteArray cmd = "CHKG ";
    cmd += escape(group);
    cmd += '\n';
    if (command(cmd) == -1) {
        return false;
    }
    return true;
}

int Client::delVar(const QByteArray &key)
{
    QByteArray cmd = "DELV ";
    cmd += escape(key);
    cmd += '\n';
    return command(cmd);
}

int Client::delGroup(const QByteArray &group)
{
    QByteArray cmd = "DELG ";
    cmd += escape(group);
    cmd += '\n';
    return command(cmd);
}

int Client::delVars(const QByteArray &special_key)
{
    QByteArray cmd = "DELS ";
    cmd += escape(special_key);
    cmd += '\n';
    return command(cmd);
}

int Client::ping()
{
    return command("PING\n");
}

int Client::exitCode()
{
    QByteArray result;
    if (command("EXIT\n", &result) != 0) {
        return -1;
    }

    return result.toInt();
}

int Client::stopServer()
{
    return command("STOP\n");
}

static QString findDaemon()
{
    QString daemon = QFile::decodeName(KDE_INSTALL_FULL_LIBEXECDIR_KF "/kdesud");
    if (!QFile::exists(daemon)) { // if not in libexec, find it in PATH
        daemon = QStandardPaths::findExecutable(QStringLiteral("kdesud"));
        if (daemon.isEmpty()) {
            qCWarning(KSU_LOG) << "kdesud daemon not found.";
        }
    }
    return daemon;
}

int Client::startServer()
{
    if (d->daemon.isEmpty()) {
        d->daemon = findDaemon();
    }
    if (d->daemon.isEmpty()) {
        return -1;
    }

    QProcess proc;
    proc.start(d->daemon, QStringList{});
    if (!proc.waitForFinished()) {
        qCCritical(KSU_LOG) << "Couldn't start kdesud!";
        return -1;
    }

    connect();
    return proc.exitCode();
}

} // namespace KDESu
