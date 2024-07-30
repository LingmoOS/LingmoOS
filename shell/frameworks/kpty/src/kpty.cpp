/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2002 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2002-2003, 2007-2008 Oswald Buddenhagen <ossi@kde.org>
    SPDX-FileCopyrightText: 2010 KDE e.V. <kde-ev-board@kde.org>
    SPDX-FileContributor: 2010 Adriaan de Groot <groot@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpty_p.h"

#include <QProcess>
#include <kpty_debug.h>

// __USE_XOPEN isn't defined by default in ICC
// (needed for ptsname(), grantpt() and unlockpt())
#ifdef __INTEL_COMPILER
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#endif

#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <cerrno>
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#if HAVE_PTY_H
#include <pty.h>
#endif

#if HAVE_LIBUTIL_H
#include <libutil.h>
#elif HAVE_UTIL_H
#include <util.h>
#endif

#ifdef UTEMPTER_PATH
// utempter uses 'add' and 'del' whereas ulog-helper uses 'login' and 'logout'
#ifndef UTEMPTER_ULOG
#define UTEMPTER_ADD "add"
#define UTEMPTER_DEL "del"
#else
#define UTEMPTER_ADD "login"
#define UTEMPTER_DEL "logout"
#endif
class UtemptProcess : public QProcess
{
public:
    UtemptProcess()
    {
        setChildProcessModifier([this]() {
            // These are the file descriptors the utempter helper wants
            dup2(cmdFd, 0);
            dup2(cmdFd, 1);
            dup2(cmdFd, 3);
        });
    }

    int cmdFd;
};
#else
#include <utmp.h>
#if HAVE_UTMPX
#include <utmpx.h>
#endif
#if !defined(_PATH_UTMPX) && defined(_UTMPX_FILE)
#define _PATH_UTMPX _UTMPX_FILE
#endif
#if !defined(_PATH_WTMPX) && defined(_WTMPX_FILE)
#define _PATH_WTMPX _WTMPX_FILE
#endif
#endif

/* for HP-UX (some versions) the extern C is needed, and for other
   platforms it doesn't hurt */
extern "C" {
#include <termios.h>
#if HAVE_TERMIO_H
#include <termio.h> // struct winsize on some systems
#endif
}

#if defined(_HPUX_SOURCE)
#define _TERMIOS_INCLUDED
#include <bsdtty.h>
#endif

#if HAVE_SYS_STROPTS_H
#include <sys/stropts.h> // Defines I_PUSH
#define _NEW_TTY_CTRL
#endif

#if HAVE_TCGETATTR
#define _tcgetattr(fd, ttmode) tcgetattr(fd, ttmode)
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__APPLE__) || defined(__DragonFly__)
#define _tcgetattr(fd, ttmode) ioctl(fd, TIOCGETA, (char *)ttmode)
#else
#define _tcgetattr(fd, ttmode) ioctl(fd, TCGETS, (char *)ttmode)
#endif

#if HAVE_TCSETATTR
#define _tcsetattr(fd, ttmode) tcsetattr(fd, TCSANOW, ttmode)
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__APPLE__) || defined(__DragonFly__)
#define _tcsetattr(fd, ttmode) ioctl(fd, TIOCSETA, (char *)ttmode)
#else
#define _tcsetattr(fd, ttmode) ioctl(fd, TCSETS, (char *)ttmode)
#endif

#include <qplatformdefs.h>

#define TTY_GROUP "tty"

#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif

///////////////////////
// private functions //
///////////////////////

//////////////////
// private data //
//////////////////

KPtyPrivate::KPtyPrivate(KPty *parent)
    : masterFd(-1)
    , slaveFd(-1)
    , ownMaster(true)
    , q_ptr(parent)
{
#ifdef UTEMPTER_PATH
    utempterPath = QStringLiteral(UTEMPTER_PATH);
#endif
}

KPtyPrivate::~KPtyPrivate()
{
}

#if !HAVE_OPENPTY
bool KPtyPrivate::chownpty(bool grant)
{
    return !QProcess::execute(QFile::decodeName(CMAKE_INSTALL_PREFIX "/" KDE_INSTALL_LIBEXECDIR_KF "/kgrantpty"),
                              QStringList() << (grant ? "--grant" : "--revoke") << QString::number(masterFd));
}
#endif

/////////////////////////////
// public member functions //
/////////////////////////////

KPty::KPty()
    : d_ptr(new KPtyPrivate(this))
{
}

KPty::KPty(KPtyPrivate *d)
    : d_ptr(d)
{
    d_ptr->q_ptr = this;
}

KPty::~KPty()
{
    close();
}

bool KPty::open()
{
    Q_D(KPty);

    if (d->masterFd >= 0) {
        return true;
    }

    d->ownMaster = true;

    QByteArray ptyName;

    // Find a master pty that we can open ////////////////////////////////

    // Because not all the pty animals are created equal, they want to
    // be opened by several different methods.

    // We try, as we know them, one by one.

#if HAVE_OPENPTY

    char ptsn[PATH_MAX];
    if (::openpty(&d->masterFd, &d->slaveFd, ptsn, nullptr, nullptr)) {
        d->masterFd = -1;
        d->slaveFd = -1;
        qCWarning(KPTY_LOG) << "Can't open a pseudo teletype";
        return false;
    }
    d->ttyName = ptsn;

#else

#if HAVE_PTSNAME || defined(TIOCGPTN)

#if HAVE_POSIX_OPENPT
    d->masterFd = ::posix_openpt(O_RDWR | O_NOCTTY);
#elif HAVE_GETPT
    d->masterFd = ::getpt();
#elif defined(PTM_DEVICE)
    d->masterFd = QT_OPEN(PTM_DEVICE, QT_OPEN_RDWR | O_NOCTTY);
#else
#error No method to open a PTY master detected.
#endif
    if (d->masterFd >= 0) {
#if HAVE_PTSNAME
        char *ptsn = ptsname(d->masterFd);
        if (ptsn) {
            d->ttyName = ptsn;
#else
        int ptyno;
        if (!ioctl(d->masterFd, TIOCGPTN, &ptyno)) {
            char buf[32];
            sprintf(buf, "/dev/pts/%d", ptyno);
            d->ttyName = buf;
#endif
#if HAVE_GRANTPT
            if (!grantpt(d->masterFd)) {
                goto grantedpt;
            }
#else
            goto gotpty;
#endif
        }
        ::close(d->masterFd);
        d->masterFd = -1;
    }
#endif // HAVE_PTSNAME || TIOCGPTN

    // Linux device names, FIXME: Trouble on other systems?
    for (const char *s3 = "pqrstuvwxyzabcde"; *s3; s3++) {
        for (const char *s4 = "0123456789abcdef"; *s4; s4++) {
            ptyName = QString().sprintf("/dev/pty%c%c", *s3, *s4).toLatin1();
            d->ttyName = QString().sprintf("/dev/tty%c%c", *s3, *s4).toLatin1();

            d->masterFd = QT_OPEN(ptyName.data(), QT_OPEN_RDWR);
            if (d->masterFd >= 0) {
                if (!access(d->ttyName.data(), R_OK | W_OK)) { // checks availability based on permission bits
                    if (!geteuid()) {
                        struct group *p = getgrnam(TTY_GROUP);
                        if (!p) {
                            p = getgrnam("wheel");
                        }
                        gid_t gid = p ? p->gr_gid : getgid();

                        chown(d->ttyName.data(), getuid(), gid);
                        chmod(d->ttyName.data(), S_IRUSR | S_IWUSR | S_IWGRP);
                    }
                    goto gotpty;
                }
                ::close(d->masterFd);
                d->masterFd = -1;
            }
        }
    }

    qCWarning(KPTY_LOG) << "Can't open a pseudo teletype";
    return false;

gotpty:
    QFileInfo info(d->ttyName.data());
    if (!info.exists()) {
        return false; // this just cannot happen ... *cough*  Yeah right, I just
    }
    // had it happen when pty #349 was allocated.  I guess
    // there was some sort of leak?  I only had a few open.
    constexpr auto perms = QFile::ReadGroup | QFile::ExeGroup | QFile::ReadOther | QFile::WriteOther | QFile::ExeOther;
    if ((info.ownerId() != getuid() || (info.permissions() & perms)) //
        && !d->chownpty(true)) {
        qCWarning(KPTY_LOG) << "chownpty failed for device " << ptyName << "::" << d->ttyName << "\nThis means the communication can be eavesdropped." << endl;
    }

grantedpt:

#ifdef HAVE_REVOKE
    revoke(d->ttyName.data());
#endif

#ifdef HAVE_UNLOCKPT
    unlockpt(d->masterFd);
#elif defined(TIOCSPTLCK)
    int flag = 0;
    ioctl(d->masterFd, TIOCSPTLCK, &flag);
#endif

    d->slaveFd = QT_OPEN(d->ttyName.data(), QT_OPEN_RDWR | O_NOCTTY);
    if (d->slaveFd < 0) {
        qCWarning(KPTY_LOG) << "Can't open slave pseudo teletype";
        ::close(d->masterFd);
        d->masterFd = -1;
        return false;
    }

#endif /* HAVE_OPENPTY */

    fcntl(d->masterFd, F_SETFD, FD_CLOEXEC);
    fcntl(d->slaveFd, F_SETFD, FD_CLOEXEC);

    return true;
}

bool KPty::open(int fd)
{
#if !HAVE_PTSNAME && !defined(TIOCGPTN)
    qCWarning(KPTY_LOG) << "Unsupported attempt to open pty with fd" << fd;
    return false;
#else
    Q_D(KPty);

    if (d->masterFd >= 0) {
        qCWarning(KPTY_LOG) << "Attempting to open an already open pty";
        return false;
    }

    d->ownMaster = false;

#if HAVE_PTSNAME
    char *ptsn = ptsname(fd);
    if (ptsn) {
        d->ttyName = ptsn;
#else
    int ptyno;
    if (!ioctl(fd, TIOCGPTN, &ptyno)) {
        char buf[32];
        sprintf(buf, "/dev/pts/%d", ptyno);
        d->ttyName = buf;
#endif
    } else {
        qCWarning(KPTY_LOG) << "Failed to determine pty slave device for fd" << fd;
        return false;
    }

    d->masterFd = fd;
    if (!openSlave()) {
        d->masterFd = -1;
        return false;
    }

    return true;
#endif
}

void KPty::closeSlave()
{
    Q_D(KPty);

    if (d->slaveFd < 0) {
        return;
    }
    ::close(d->slaveFd);
    d->slaveFd = -1;
}

bool KPty::openSlave()
{
    Q_D(KPty);

    if (d->slaveFd >= 0) {
        return true;
    }
    if (d->masterFd < 0) {
        qCWarning(KPTY_LOG) << "Attempting to open pty slave while master is closed";
        return false;
    }
    d->slaveFd = QT_OPEN(d->ttyName.data(), QT_OPEN_RDWR | O_NOCTTY);
    if (d->slaveFd < 0) {
        qCWarning(KPTY_LOG) << "Can't open slave pseudo teletype";
        return false;
    }
    fcntl(d->slaveFd, F_SETFD, FD_CLOEXEC);
    return true;
}

void KPty::close()
{
    Q_D(KPty);

    if (d->masterFd < 0) {
        return;
    }
    closeSlave();
    if (d->ownMaster) {
#if !HAVE_OPENPTY
        // don't bother resetting unix98 pty, it will go away after closing master anyway.
        if (memcmp(d->ttyName.data(), "/dev/pts/", 9)) {
            if (!geteuid()) {
                struct stat st;
                if (!stat(d->ttyName.data(), &st)) {
                    chown(d->ttyName.data(), 0, st.st_gid == getgid() ? 0 : -1);
                    chmod(d->ttyName.data(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
                }
            } else {
                fcntl(d->masterFd, F_SETFD, 0);
                d->chownpty(false);
            }
        }
#endif
        ::close(d->masterFd);
    }
    d->masterFd = -1;
}

void KPty::setCTty()
{
    Q_D(KPty);

    if (!d->withCTty) {
        return;
    }

    // Setup job control //////////////////////////////////

    // Become session leader, process group leader,
    // and get rid of the old controlling terminal.
    setsid();

    // make our slave pty the new controlling terminal.
    ioctl(d->slaveFd, TIOCSCTTY, 0);

    // make our new process group the foreground group on the pty
    int pgrp = getpid();
    tcsetpgrp(d->slaveFd, pgrp);
}

void KPty::login(const char *user, const char *remotehost)
{
#ifdef UTEMPTER_PATH
    Q_D(KPty);

    Q_UNUSED(user);

    // Emulating libutempter version 1.1.6
    if (!d->utempterPath.isEmpty()) {
        UtemptProcess utemptProcess;
        utemptProcess.cmdFd = d->masterFd;
        utemptProcess.setProgram(d->utempterPath);
        utemptProcess.setArguments(QStringList() << QStringLiteral(UTEMPTER_ADD) << QString::fromLocal8Bit(remotehost));
        utemptProcess.setProcessChannelMode(QProcess::ForwardedChannels);
        utemptProcess.start();
        utemptProcess.waitForFinished();
    }

#else
#if HAVE_UTMPX
    struct utmpx l_struct;
#else
    struct utmp l_struct;
#endif
    memset(&l_struct, 0, sizeof(l_struct));
    // note: strncpy without terminators _is_ correct here. man 4 utmp

    if (user) {
        strncpy(l_struct.ut_name, user, sizeof(l_struct.ut_name));
    }

    if (remotehost) {
        strncpy(l_struct.ut_host, remotehost, sizeof(l_struct.ut_host));
#if HAVE_STRUCT_UTMP_UT_SYSLEN
        l_struct.ut_syslen = qMin(strlen(remotehost), sizeof(l_struct.ut_host));
#endif
    }

#ifndef __GLIBC__
    Q_D(KPty);
    const char *str_ptr = d->ttyName.data();
    if (!memcmp(str_ptr, "/dev/", 5)) {
        str_ptr += 5;
    }
    strncpy(l_struct.ut_line, str_ptr, sizeof(l_struct.ut_line));
#if HAVE_STRUCT_UTMP_UT_ID
    strncpy(l_struct.ut_id, str_ptr + strlen(str_ptr) - sizeof(l_struct.ut_id), sizeof(l_struct.ut_id));
#endif
#endif

#if HAVE_UTMPX
    gettimeofday(&l_struct.ut_tv, 0);
#else
    l_struct.ut_time = time(0);
#endif

#if HAVE_LOGIN
#if HAVE_LOGINX
    ::loginx(&l_struct);
#else
    ::login(&l_struct);
#endif
#else
#if HAVE_STRUCT_UTMP_UT_TYPE
    l_struct.ut_type = USER_PROCESS;
#endif
#if HAVE_STRUCT_UTMP_UT_PID
    l_struct.ut_pid = getpid();
#if HAVE_STRUCT_UTMP_UT_SESSION
    l_struct.ut_session = getsid(0);
#endif
#endif
#if HAVE_UTMPX
    utmpxname(_PATH_UTMPX);
    setutxent();
    pututxline(&l_struct);
    endutxent();
    updwtmpx(_PATH_WTMPX, &l_struct);
#else
    utmpname(_PATH_UTMP);
    setutent();
    pututline(&l_struct);
    endutent();
    updwtmp(_PATH_WTMP, &l_struct);
#endif
#endif
#endif
}

void KPty::logout()
{
#ifdef UTEMPTER_PATH
    Q_D(KPty);

    // Emulating libutempter version 1.1.6
    if (!d->utempterPath.isEmpty()) {
        UtemptProcess utemptProcess;
        utemptProcess.cmdFd = d->masterFd;
        utemptProcess.setProgram(d->utempterPath);
        utemptProcess.setArguments(QStringList(QStringLiteral(UTEMPTER_DEL)));
        utemptProcess.setProcessChannelMode(QProcess::ForwardedChannels);
        utemptProcess.start();
        utemptProcess.waitForFinished();
    }

#else
    Q_D(KPty);

    const char *str_ptr = d->ttyName.data();
    if (!memcmp(str_ptr, "/dev/", 5)) {
        str_ptr += 5;
    }
#ifdef __GLIBC__
    else {
        const char *sl_ptr = strrchr(str_ptr, '/');
        if (sl_ptr) {
            str_ptr = sl_ptr + 1;
        }
    }
#endif
#if HAVE_LOGIN
#if HAVE_LOGINX
    ::logoutx(str_ptr, 0, DEAD_PROCESS);
#else
    ::logout(str_ptr);
#endif
#else
#if HAVE_UTMPX
    struct utmpx l_struct, *ut;
#else
    struct utmp l_struct, *ut;
#endif
    memset(&l_struct, 0, sizeof(l_struct));

    strncpy(l_struct.ut_line, str_ptr, sizeof(l_struct.ut_line));

#if HAVE_UTMPX
    utmpxname(_PATH_UTMPX);
    setutxent();
    if ((ut = getutxline(&l_struct))) {
#else
    utmpname(_PATH_UTMP);
    setutent();
    if ((ut = getutline(&l_struct))) {
#endif
        memset(ut->ut_name, 0, sizeof(*ut->ut_name));
        memset(ut->ut_host, 0, sizeof(*ut->ut_host));
#if HAVE_STRUCT_UTMP_UT_SYSLEN
        ut->ut_syslen = 0;
#endif
#if HAVE_STRUCT_UTMP_UT_TYPE
        ut->ut_type = DEAD_PROCESS;
#endif
#if HAVE_UTMPX
        gettimeofday(&(ut->ut_tv), 0);
        pututxline(ut);
    }
    endutxent();
#else
        ut->ut_time = time(0);
        pututline(ut);
    }
    endutent();
#endif
#endif
#endif
}

bool KPty::tcGetAttr(struct ::termios *ttmode) const
{
    Q_D(const KPty);

    return _tcgetattr(d->masterFd, ttmode) == 0;
}

bool KPty::tcSetAttr(struct ::termios *ttmode)
{
    Q_D(KPty);

    return _tcsetattr(d->masterFd, ttmode) == 0;
}

bool KPty::setWinSize(int lines, int columns, int height, int width)
{
    Q_D(KPty);

    struct winsize winSize;
    winSize.ws_row = (unsigned short)lines;
    winSize.ws_col = (unsigned short)columns;
    winSize.ws_ypixel = (unsigned short)height;
    winSize.ws_xpixel = (unsigned short)width;
    return ioctl(d->masterFd, TIOCSWINSZ, (char *)&winSize) == 0;
}

bool KPty::setWinSize(int lines, int columns)
{
    return setWinSize(lines, columns, 0, 0);
}

bool KPty::setEcho(bool echo)
{
    struct ::termios ttmode;
    if (!tcGetAttr(&ttmode)) {
        return false;
    }
    if (!echo) {
        ttmode.c_lflag &= ~ECHO;
    } else {
        ttmode.c_lflag |= ECHO;
    }
    return tcSetAttr(&ttmode);
}

const char *KPty::ttyName() const
{
    Q_D(const KPty);

    return d->ttyName.data();
}

int KPty::masterFd() const
{
    Q_D(const KPty);

    return d->masterFd;
}

int KPty::slaveFd() const
{
    Q_D(const KPty);

    return d->slaveFd;
}

void KPty::setCTtyEnabled(bool enable)
{
    Q_D(KPty);

    d->withCTty = enable;
}
