/* vi: ts=8 sts=4 sw=4

    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>


    kdesud.cpp: KDE su daemon. Offers "keep password" functionality to kde su.

    The socket $KDEHOME/socket-$(HOSTNAME)/kdesud_$(display) is used for communication with
    client programs.

    The protocol: Client initiates the connection. All commands and responses
    are terminated by a newline.

    Client                     Server     Description
    ------                     ------     -----------

    PASS <pass> <timeout>      OK         Set password for commands in
                                          this session. Password is
                                          valid for <timeout> seconds.

    USER <user>                OK         Set the target user [required]

    EXEC <command>             OK         Execute command <command>. If
                               NO         <command> has been executed
                                          before (< timeout) no PASS
                                          command is needed.

    DEL <command>              OK         Delete password for command
                               NO         <command>.

    PING                       OK         Ping the server (diagnostics).
*/

#include "config-kdesud.h"
#include <config-kdesu.h>
#include <ksud_debug.h>

#include <cerrno>
#include <ctype.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h> // Needed on some systems.
#endif

#include <dirent.h>

#if !HAVE_CLOSE_RANGE
#include <sys/syscall.h> // close_range syscall
#endif

#include <QByteArray>
#include <QCommandLineParser>
#include <QFile>
#include <QList>
#include <QRegularExpression>
#include <QStandardPaths>

#include <KAboutData>
#include <KLocalizedString>
#include <client.h>
#include <defaults.h>

#include "handler.h"
#include "repo.h"

#if HAVE_X11
#include <X11/X.h>
#include <X11/Xlib.h>
#endif

#ifdef __FreeBSD__
#include <sys/procctl.h>
#elif defined(__linux__)
#include <sys/prctl.h>
#endif

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((socklen_t)(offsetof(struct sockaddr_un, sun_path) + strlen((ptr)->sun_path)))
#endif

#define ERR strerror(errno)

using namespace KDESu;

static int closeExtraFds()
{
#if HAVE_CLOSE_RANGE
    const int res = close_range(4, ~0U, 0);
    if (res == 0) {
        return 0;
    }
    // If ENOSYS, fallback to opendir/readdir/closedir below
    if (errno != ENOSYS) {
        return -1;
    }
#elif defined(SYS_close_range)
    const int res = syscall(SYS_close_range, 4, ~0U, 0);
    if (res == 0) {
        return 0;
    }
    // If ENOSYS, fallback to opendir/readdir/closedir below
    if (errno != ENOSYS) {
        return -1;
    }
#endif

#if !defined(__FreeBSD__) && !defined(__OpenBSD__) // /proc, /dev are Linux only
    // close_range isn't available, fallback to iterarting over "/dev/fd/"
    // and close the fd's manually
    qCDebug(KSUD_LOG) << "close_range function/syscall isn't available, falling back to iterarting "
                         "over '/dev/fd' and closing the file descriptors manually.\n";

    std::unique_ptr<DIR, int (*)(DIR *)> dirPtr(opendir("/dev/fd"), closedir);
    if (!dirPtr) {
        return -1;
    }

    int closeRes = 0;
    const int dirFd = dirfd(dirPtr.get());
    while (struct dirent *dirEnt = readdir(dirPtr.get())) {
        const int currFd = std::atoi(dirEnt->d_name);
        if (currFd > 3 && currFd != dirFd) {
            closeRes = close(currFd);
            if (closeRes == -1) {
                break;
            }
        }
    }
    return closeRes;
#else
    return -1;
#endif
}

// Globals

Repository *repo;
QString Version(QStringLiteral("1.01"));
QByteArray sock;
#if HAVE_X11
Display *x11Display;
#endif
int pipeOfDeath[2];

void kdesud_cleanup()
{
    unlink(sock.constData());
}

// Borrowed from kdebase/kaudio/kaudioserver.cpp

#if HAVE_X11
extern "C" int xio_errhandler(Display *);

int xio_errhandler(Display *)
{
    qCCritical(KSUD_LOG) << "Fatal IO error, exiting...\n";
    kdesud_cleanup();
    exit(1);
    return 1; // silence compilers
}

int initXconnection()
{
    x11Display = XOpenDisplay(nullptr);
    if (x11Display != nullptr) {
        XSetIOErrorHandler(xio_errhandler);
        /* clang-format off */
        XCreateSimpleWindow(x11Display,
                            DefaultRootWindow(x11Display),
                            0, 0, 1, 1, 0,
                            BlackPixelOfScreen(DefaultScreenOfDisplay(x11Display)),
                            BlackPixelOfScreen(DefaultScreenOfDisplay(x11Display)));
        /* clang-format on*/
        return XConnectionNumber(x11Display);
    } else {
        qCWarning(KSUD_LOG) << "Can't connect to the X Server.\n";
        qCWarning(KSUD_LOG) << "Might not terminate at end of session.\n";
        return -1;
    }
}
#endif

extern "C" {
void signal_exit(int);
void sigchld_handler(int);
}

void signal_exit(int sig)
{
    qCDebug(KSUD_LOG) << "Exiting on signal " << sig << "\n";
    kdesud_cleanup();
    exit(1);
}

void sigchld_handler(int)
{
    char c = ' ';
    write(pipeOfDeath[1], &c, 1);
}

/**
 * Creates an AF_UNIX socket in socket resource, mode 0600.
 */

int create_socket()
{
    int sockfd;
    socklen_t addrlen;
    struct stat s;

    QString display = QString::fromLocal8Bit(qgetenv("DISPLAY"));
    if (display.isEmpty()) {
        qCWarning(KSUD_LOG) << "$DISPLAY is not set\n";
        return -1;
    }

    // strip the screen number from the display
    display.remove(QRegularExpression(QStringLiteral("\\.[0-9]+$")));

    sock = QFile::encodeName(QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation) + QStringLiteral("/kdesud_%1").arg(display));
    int stat_err = lstat(sock.constData(), &s);
    if (!stat_err && S_ISLNK(s.st_mode)) {
        qCWarning(KSUD_LOG) << "Someone is running a symlink attack on you\n";
        if (unlink(sock.constData())) {
            qCWarning(KSUD_LOG) << "Could not delete symlink\n";
            return -1;
        }
    }

    if (!access(sock.constData(), R_OK | W_OK)) {
        KDESu::Client client;
        if (client.ping() == -1) {
            qCWarning(KSUD_LOG) << "stale socket exists\n";
            if (unlink(sock.constData())) {
                qCWarning(KSUD_LOG) << "Could not delete stale socket\n";
                return -1;
            }
        } else {
            qCWarning(KSUD_LOG) << "kdesud is already running\n";
            return -1;
        }
    }

    sockfd = socket(PF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (sockfd < 0) {
        qCCritical(KSUD_LOG) << "socket(): " << ERR << "\n";
        return -1;
    }

    // Ensure socket closed on error
    struct fd_ScopeGuard {
        fd_ScopeGuard(int fd)
            : _fd(fd)
        {
        }
        ~fd_ScopeGuard()
        {
            if (_fd >= 0) {
                close(_fd);
            }
        }
        fd_ScopeGuard(const fd_ScopeGuard &) = delete;
        fd_ScopeGuard &operator=(const fd_ScopeGuard &) = delete;
        void reset()
        {
            _fd = -1;
        }
        int _fd;
    } guard(sockfd);

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock.constData(), sizeof(addr.sun_path) - 1);
    addr.sun_path[sizeof(addr.sun_path) - 1] = '\000';
    addrlen = SUN_LEN(&addr);
    if (bind(sockfd, (struct sockaddr *)&addr, addrlen) < 0) {
        qCCritical(KSUD_LOG) << "bind(): " << ERR << "\n";
        return -1;
    }

    struct linger lin;
    lin.l_onoff = lin.l_linger = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof(linger)) < 0) {
        qCCritical(KSUD_LOG) << "setsockopt(SO_LINGER): " << ERR << "\n";
        return -1;
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        qCCritical(KSUD_LOG) << "setsockopt(SO_REUSEADDR): " << ERR << "\n";
        return -1;
    }
    opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&opt, sizeof(opt)) < 0) {
        qCCritical(KSUD_LOG) << "setsockopt(SO_KEEPALIVE): " << ERR << "\n";
        return -1;
    }
    chmod(sock.constData(), 0600);
    guard.reset();
    return sockfd;
}
/* The daemon stores passwords, which we don't want any other process to be able to read. */
static bool prevent_tracing()
{
    int r = -1;
#ifdef PR_SET_DUMPABLE
    // Linux
    r = prctl(PR_SET_DUMPABLE, 0, 0, 0, 0);
#elif defined(PROC_TRACE_CTL)
    // FreeBSD
    int disable = PROC_TRACE_CTL_DISABLE_EXEC;
    r = procctl(P_PID, 0, PROC_TRACE_CTL, &disable);
#else
#warning Missing implementation for disabling traceability on this platform
#endif

    return r == 0;
}

/**
 * Main program
 */

int main(int argc, char *argv[])
{
    if (!prevent_tracing()) {
        qCWarning(KSUD_LOG) << "failed to make process memory untraceable" << strerror(errno);
    }

    QCoreApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("kdesud") /* componentName */,
                         i18n("KDE su daemon"),
                         Version,
                         i18n("Daemon used by kdesu"),
                         KAboutLicense::Artistic,
                         i18n("Copyright (c) 1999,2000 Geert Jansen"));
    aboutData.addAuthor(i18n("Geert Jansen"), i18n("Author"), QStringLiteral("jansen@kde.org"), QStringLiteral("http://www.stack.nl/~geertj/"));

    KAboutData::setApplicationData(aboutData);
    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    // Set core dump size to 0
    struct rlimit rlim;
    rlim.rlim_cur = rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_CORE, &rlim) < 0) {
        qCCritical(KSUD_LOG) << "setrlimit(): " << ERR << "\n";
        exit(1);
    }

    // Create the Unix socket.
    int sockfd = create_socket();
    if (sockfd < 0) {
        exit(1);
    }
    if (listen(sockfd, 10) < 0) {
        qCCritical(KSUD_LOG) << "listen(): " << ERR << "\n";
        kdesud_cleanup();
        exit(1);
    }

    if (sockfd != 3) {
        sockfd = dup3(sockfd, 3, O_CLOEXEC);
    }
    if (sockfd < 0) {
        qCCritical(KSUD_LOG) << "Failed to set sockfd to fd 3" << ERR << "\n";
        kdesud_cleanup();
        exit(1);
    }

    int maxfd = sockfd;

    if (closeExtraFds() < 0) {
        qCCritical(KSUD_LOG) << "Failed to close file descriptors higher than 3, with error:" << ERR << "\n";
        kdesud_cleanup();
        exit(1);
    }

    // Ok, we're accepting connections. Fork to the background.
    pid_t pid = fork();
    if (pid == -1) {
        qCCritical(KSUD_LOG) << "fork():" << ERR << "\n";
        kdesud_cleanup();
        exit(1);
    }
    if (pid) {
        _exit(0);
    }

#if HAVE_X11
    // Make sure we exit when the display gets closed.
    int x11Fd = initXconnection();
    maxfd = qMax(maxfd, x11Fd);
#endif

    repo = new Repository;
    QList<ConnectionHandler *> handler;

    pipe2(pipeOfDeath, O_CLOEXEC);
    maxfd = qMax(maxfd, pipeOfDeath[0]);

    // Signal handlers
    struct sigaction sa;
    sa.sa_handler = signal_exit;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGHUP, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGQUIT, &sa, nullptr);

    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, nullptr);
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, nullptr);

    // Main execution loop

    socklen_t addrlen;
    struct sockaddr_un clientname;

    fd_set tmp_fds;
    fd_set active_fds;
    FD_ZERO(&active_fds);
    FD_SET(sockfd, &active_fds);
    FD_SET(pipeOfDeath[0], &active_fds);
#if HAVE_X11
    if (x11Fd != -1) {
        FD_SET(x11Fd, &active_fds);
    }
#endif

    while (1) {
        tmp_fds = active_fds;
#if HAVE_X11
        if (x11Display) {
            XFlush(x11Display);
        }
#endif
        if (select(maxfd + 1, &tmp_fds, nullptr, nullptr, nullptr) < 0) {
            if (errno == EINTR) {
                continue;
            }

            qCCritical(KSUD_LOG) << "select(): " << ERR << "\n";
            exit(1);
        }
        repo->expire();
        for (int i = 0; i <= maxfd; i++) {
            if (!FD_ISSET(i, &tmp_fds)) {
                continue;
            }

            if (i == pipeOfDeath[0]) {
                char buf[101];
                read(pipeOfDeath[0], buf, 100);
                pid_t result;
                do {
                    int status;
                    result = waitpid((pid_t)-1, &status, WNOHANG);
                    if (result > 0) {
                        for (int j = handler.size(); j--;) {
                            if (handler[j] && (handler[j]->m_pid == result)) {
                                handler[j]->m_exitCode = WEXITSTATUS(status);
                                handler[j]->m_hasExitCode = true;
                                handler[j]->sendExitCode();
                                handler[j]->m_pid = 0;
                                break;
                            }
                        }
                    }
                } while (result > 0);
            }

#if HAVE_X11
            if (i == x11Fd) {
                // Discard X events
                XEvent event_return;
                if (x11Display) {
                    while (XPending(x11Display)) {
                        XNextEvent(x11Display, &event_return);
                    }
                }
                continue;
            }
#endif

            if (i == sockfd) {
                // Accept new connection
                int fd;
                addrlen = 64;
                fd = accept(sockfd, (struct sockaddr *)&clientname, &addrlen);
                if (fd < 0) {
                    qCCritical(KSUD_LOG) << "accept():" << ERR << "\n";
                    continue;
                }
                while (fd + 1 > (int)handler.size()) {
                    handler.append(nullptr);
                }
                delete handler[fd];
                handler[fd] = new ConnectionHandler(fd);
                maxfd = qMax(maxfd, fd);
                FD_SET(fd, &active_fds);
                fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
                continue;
            }

            // handle already established connection
            if (handler[i] && handler[i]->handle() < 0) {
                delete handler[i];
                handler[i] = nullptr;
                FD_CLR(i, &active_fds);
            }
        }
    }
    qCWarning(KSUD_LOG) << "???\n";
}
