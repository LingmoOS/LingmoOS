/*
    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    This file contains code from TEShell.C of the KDE konsole.
    SPDX-FileCopyrightText: 1997, 1998 Lars Doelle <lars.doelle@on-line.de>

    SPDX-License-Identifier: GPL-2.0-only

    process.cpp: Functionality to build a front end to password asking terminal programs.
*/

#include "ptyprocess.h"
#include "kcookie_p.h"
#include "ptyprocess_p.h"

#include <config-kdesu.h>
#include <ksu_debug.h>

#include <cerrno>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

#if HAVE_SYS_SELECT_H
#include <sys/select.h> // Needed on some systems.
#endif

#include <QFile>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KSharedConfig>

extern int kdesuDebugArea();

namespace KDESu
{
using namespace KDESuPrivate;

/*
** Wait for @p ms milliseconds
** @param fd file descriptor
** @param ms time to wait in milliseconds
** @return
*/
int PtyProcess::waitMS(int fd, int ms)
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000 * ms;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    return select(fd + 1, &fds, nullptr, nullptr, &tv);
}

// XXX this function is nonsense:
// - for our child, we could use waitpid().
// - the configurability at this place it *complete* braindamage
/*
** Basic check for the existence of @p pid.
** Returns true iff @p pid is an extant process.
*/
bool PtyProcess::checkPid(pid_t pid)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, QStringLiteral("super-user-command"));
    QString superUserCommand = cg.readEntry("super-user-command", "sudo");
    // sudo does not accept signals from user so we except it
    if (superUserCommand == QLatin1String("sudo")) {
        return true;
    } else {
        return kill(pid, 0) == 0;
    }
}

/*
** Check process exit status for process @p pid.
** On error (no child, no exit), return Error (-1).
** If child @p pid has exited, return its exit status,
** (which may be zero).
** If child @p has not exited, return NotExited (-2).
*/
int PtyProcess::checkPidExited(pid_t pid)
{
    int state;
    int ret;
    ret = waitpid(pid, &state, WNOHANG);

    if (ret < 0) {
        qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                            << "waitpid():" << strerror(errno);
        return Error;
    }
    if (ret == pid) {
        if (WIFEXITED(state)) {
            return WEXITSTATUS(state);
        }

        return Killed;
    }

    return NotExited;
}

PtyProcess::PtyProcess()
    : PtyProcess(*new PtyProcessPrivate)
{
}

PtyProcess::PtyProcess(PtyProcessPrivate &dd)
    : d_ptr(&dd)
{
    m_terminal = false;
    m_erase = false;
}

PtyProcess::~PtyProcess() = default;

int PtyProcess::init()
{
    Q_D(PtyProcess);

    delete d->pty;
    d->pty = new KPty();
    if (!d->pty->open()) {
        qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                            << "Failed to open PTY.";
        return -1;
    }
    if (!d->wantLocalEcho) {
        enableLocalEcho(false);
    }
    d->inputBuffer.resize(0);
    return 0;
}

/** Set additional environment variables. */
void PtyProcess::setEnvironment(const QList<QByteArray> &env)
{
    Q_D(PtyProcess);

    d->env = env;
}

int PtyProcess::fd() const
{
    Q_D(const PtyProcess);

    return d->pty ? d->pty->masterFd() : -1;
}

int PtyProcess::pid() const
{
    return m_pid;
}

/** Returns the additional environment variables set by setEnvironment() */
QList<QByteArray> PtyProcess::environment() const
{
    Q_D(const PtyProcess);

    return d->env;
}

QByteArray PtyProcess::readAll(bool block)
{
    Q_D(PtyProcess);

    QByteArray ret;
    if (!d->inputBuffer.isEmpty()) {
        // if there is still something in the buffer, we need not block.
        // we should still try to read any further output, from the fd, though.
        block = false;
        ret = d->inputBuffer;
        d->inputBuffer.resize(0);
    }

    int flags = fcntl(fd(), F_GETFL);
    if (flags < 0) {
        qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                            << "fcntl(F_GETFL):" << strerror(errno);
        return ret;
    }
    int oflags = flags;
    if (block) {
        flags &= ~O_NONBLOCK;
    } else {
        flags |= O_NONBLOCK;
    }

    if ((flags != oflags) && (fcntl(fd(), F_SETFL, flags) < 0)) {
        // We get an error here when the child process has closed
        // the file descriptor already.
        return ret;
    }

    while (1) {
        ret.reserve(ret.size() + 0x8000);
        int nbytes = read(fd(), ret.data() + ret.size(), 0x8000);
        if (nbytes == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        }
        if (nbytes == 0) {
            break; // nothing available / eof
        }

        ret.resize(ret.size() + nbytes);
        break;
    }

    return ret;
}

QByteArray PtyProcess::readLine(bool block)
{
    Q_D(PtyProcess);

    d->inputBuffer = readAll(block);

    int pos;
    QByteArray ret;
    if (!d->inputBuffer.isEmpty()) {
        pos = d->inputBuffer.indexOf('\n');
        if (pos == -1) {
            // NOTE: this means we return something even if there in no full line!
            ret = d->inputBuffer;
            d->inputBuffer.resize(0);
        } else {
            ret = d->inputBuffer.left(pos);
            d->inputBuffer.remove(0, pos + 1);
        }
    }

    return ret;
}

void PtyProcess::writeLine(const QByteArray &line, bool addnl)
{
    if (!line.isEmpty()) {
        write(fd(), line.constData(), line.length());
    }
    if (addnl) {
        write(fd(), "\n", 1);
    }
}

void PtyProcess::unreadLine(const QByteArray &line, bool addnl)
{
    Q_D(PtyProcess);

    QByteArray tmp = line;
    if (addnl) {
        tmp += '\n';
    }
    if (!tmp.isEmpty()) {
        d->inputBuffer.prepend(tmp);
    }
}

void PtyProcess::setExitString(const QByteArray &exit)
{
    m_exitString = exit;
}

/*
 * Fork and execute the command. This returns in the parent.
 */
int PtyProcess::exec(const QByteArray &command, const QList<QByteArray> &args)
{
    Q_D(PtyProcess);

    int i;

    if (init() < 0) {
        return -1;
    }

    if ((m_pid = fork()) == -1) {
        qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                            << "fork():" << strerror(errno);
        return -1;
    }

    // Parent
    if (m_pid) {
        d->pty->closeSlave();
        return 0;
    }

    // Child
    if (setupTTY() < 0) {
        _exit(1);
    }

    for (const QByteArray &var : std::as_const(d->env)) {
        putenv(const_cast<char *>(var.constData()));
    }
    unsetenv("KDE_FULL_SESSION");
    // for : Qt: Session management error
    unsetenv("SESSION_MANAGER");
    // QMutex::lock , deadlocks without that.
    // <thiago> you cannot connect to the user's session bus from another UID
    unsetenv("DBUS_SESSION_BUS_ADDRESS");

    // set temporarily LC_ALL to C, for su (to be able to parse "Password:")
    const QByteArray old_lc_all = qgetenv("LC_ALL");
    if (!old_lc_all.isEmpty()) {
        qputenv("KDESU_LC_ALL", old_lc_all);
    } else {
        unsetenv("KDESU_LC_ALL");
    }
    qputenv("LC_ALL", "C");

    // From now on, terminal output goes through the tty.

    QByteArray path;
    if (command.contains('/')) {
        path = command;
    } else {
        QString file = QStandardPaths::findExecutable(QFile::decodeName(command));
        if (file.isEmpty()) {
            qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] " << command << "not found.";
            _exit(1);
        }
        path = QFile::encodeName(file);
    }

    const char **argp = (const char **)malloc((args.count() + 2) * sizeof(char *));

    i = 0;
    argp[i++] = path.constData();
    for (const QByteArray &arg : args) {
        argp[i++] = arg.constData();
    }

    argp[i] = nullptr;

    execv(path.constData(), const_cast<char **>(argp));
    qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                        << "execv(" << path << "):" << strerror(errno);
    _exit(1);
    return -1; // Shut up compiler. Never reached.
}

/*
 * Wait until the terminal is set into no echo mode. At least one su
 * (RH6 w/ Linux-PAM patches) sets noecho mode AFTER writing the Password:
 * prompt, using TCSAFLUSH. This flushes the terminal I/O queues, possibly
 * taking the password  with it. So we wait until no echo mode is set
 * before writing the password.
 * Note that this is done on the slave fd. While Linux allows tcgetattr() on
 * the master side, Solaris doesn't.
 */
int PtyProcess::waitSlave()
{
    Q_D(PtyProcess);

    struct termios tio;
    while (1) {
        if (!checkPid(m_pid)) {
            qCCritical(KSU_LOG) << "process has exited while waiting for password.";
            return -1;
        }
        if (!d->pty->tcGetAttr(&tio)) {
            qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                                << "tcgetattr():" << strerror(errno);
            return -1;
        }
        if (tio.c_lflag & ECHO) {
            // qDebug() << "[" << __FILE__ << ":" << __LINE__ << "] " << "Echo mode still on.";
            usleep(10000);
            continue;
        }
        break;
    }
    return 0;
}

int PtyProcess::enableLocalEcho(bool enable)
{
    Q_D(PtyProcess);

    d->wantLocalEcho = enable;
    if (!d->pty) {
        // Apply it on init
        return 0;
    }

    return d->pty->setEcho(enable) ? 0 : -1;
}

void PtyProcess::setTerminal(bool terminal)
{
    m_terminal = terminal;
}

void PtyProcess::setErase(bool erase)
{
    m_erase = erase;
}

/*
 * Copy output to stdout until the child process exits, or a line of output
 * matches `m_exitString'.
 * We have to use waitpid() to test for exit. Merely waiting for EOF on the
 * pty does not work, because the target process may have children still
 * attached to the terminal.
 */
int PtyProcess::waitForChild()
{
    fd_set fds;
    FD_ZERO(&fds);
    QByteArray remainder;

    while (1) {
        FD_SET(fd(), &fds);

        // specify timeout to make sure select() does not block, even if the
        // process is dead / non-responsive. It does not matter if we abort too
        // early. In that case 0 is returned, and we'll try again in the next
        // iteration. (As long as we don't consistently time out in each iteration)
        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        int ret = select(fd() + 1, &fds, nullptr, nullptr, &timeout);
        if (ret == -1) {
            if (errno != EINTR) {
                qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                                    << "select():" << strerror(errno);
                return -1;
            }
            ret = 0;
        }

        if (ret) {
            for (;;) {
                QByteArray output = readAll(false);
                if (output.isEmpty()) {
                    break;
                }
                if (m_terminal) {
                    fwrite(output.constData(), output.size(), 1, stdout);
                    fflush(stdout);
                }
                if (!m_exitString.isEmpty()) {
                    // match exit string only at line starts
                    remainder += output;
                    while (remainder.length() >= m_exitString.length()) {
                        if (remainder.startsWith(m_exitString)) {
                            kill(m_pid, SIGTERM);
                            remainder.remove(0, m_exitString.length());
                        }
                        int off = remainder.indexOf('\n');
                        if (off < 0) {
                            break;
                        }
                        remainder.remove(0, off + 1);
                    }
                }
            }
        }

        ret = checkPidExited(m_pid);
        if (ret == Error) {
            if (errno == ECHILD) {
                return 0;
            } else {
                return 1;
            }
        } else if (ret == Killed) {
            return 0;
        } else if (ret == NotExited) {
            continue; // keep checking
        } else {
            return ret;
        }
    }
}

/*
 * SetupTTY: Creates a new session. The filedescriptor "fd" should be
 * connected to the tty. It is closed after the tty is reopened to make it
 * our controlling terminal. This way the tty is always opened at least once
 * so we'll never get EIO when reading from it.
 */
int PtyProcess::setupTTY()
{
    Q_D(PtyProcess);

    // Reset signal handlers
    for (int sig = 1; sig < NSIG; sig++) {
        signal(sig, SIG_DFL);
    }
    signal(SIGHUP, SIG_IGN);

    d->pty->setCTty();

    // Connect stdin, stdout and stderr
    int slave = d->pty->slaveFd();
    dup2(slave, 0);
    dup2(slave, 1);
    dup2(slave, 2);

    // Close all file handles
    // XXX this caused problems in KProcess - not sure why anymore. -- ???
    // Because it will close the start notification pipe. -- ossi
    struct rlimit rlp;
    getrlimit(RLIMIT_NOFILE, &rlp);
    for (int i = 3; i < (int)rlp.rlim_cur; i++) {
        close(i);
    }

    // Disable OPOST processing. Otherwise, '\n' are (on Linux at least)
    // translated to '\r\n'.
    struct ::termios tio;
    if (tcgetattr(0, &tio) < 0) {
        qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                            << "tcgetattr():" << strerror(errno);
        return -1;
    }
    tio.c_oflag &= ~OPOST;
    if (tcsetattr(0, TCSANOW, &tio) < 0) {
        qCCritical(KSU_LOG) << "[" << __FILE__ << ":" << __LINE__ << "] "
                            << "tcsetattr():" << strerror(errno);
        return -1;
    }

    return 0;
}

void PtyProcess::virtual_hook(int id, void *data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
    /*BASE::virtual_hook( id, data );*/
}

} // namespace KDESu
