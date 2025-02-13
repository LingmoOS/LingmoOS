/*****************************************************************
 *
 *	kcheckpass - Simple password checker
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 *	kcheckpass is a simple password checker. Just invoke and
 *      send it the password on stdin.
 *
 *	If the password was accepted, the program exits with 0;
 *	if it was rejected, it exits with 1. Any other exit
 *	code signals an error.
 *
 *	It's hopefully simple enough to allow it to be setuid
 *	root.
 *
 *	Compile with -DHAVE_VSYSLOG if you have vsyslog().
 *	Compile with -DHAVE_PAM if you have a PAM system,
 *	and link with -lpam -ldl.
 *	Compile with -DHAVE_SHADOW if you have a shadow
 *	password system.
 *
 *	Copyright (C) 1998, Caldera, Inc.
 *	Released under the GNU General Public License
 *
 *	Olaf Kirch <okir@caldera.de>         General Framework and PAM support
 *	Christian Esken <esken@kde.org>      Shadow and /etc/passwd support
 *	Roberto Teixeira <maragato@kde.org>  other user (-U) support
 *	Oswald Buddenhagen <ossi@kde.org>    Binary server mode
 *
 *      Other parts were taken from kscreensaver's passwd.cpp.
 *
 *****************************************************************/

#include "checkpass.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <config-screenlocker.h>

#if HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif
#if HAVE_SYS_PROCCTL_H
#include <sys/procctl.h>
#include <unistd.h>
#endif
#if HAVE_SIGNALFD_H
#include <sys/signalfd.h>
#endif
#if HAVE_EVENT_H
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#endif

#define THROTTLE 3

static int havetty, sfd = -1, nullpass;

static int Reader(void *buf, int count)
{
    int ret, rlen;

    for (rlen = 0; rlen < count;) {
    dord:
        ret = read(sfd, (void *)((char *)buf + rlen), count - rlen);
        if (ret < 0) {
            if (errno == EINTR) {
                goto dord;
            }
            if (errno == EAGAIN) {
                break;
            }
            return -1;
        }
        if (!ret) {
            break;
        }
        rlen += ret;
    }
    return rlen;
}

static void GRead(void *buf, int count)
{
    if (Reader(buf, count) != count) {
        message("Communication breakdown on read\n");
        exit(15);
    }
}

static void GWrite(const void *buf, int count)
{
    if (write(sfd, buf, count) != count) {
        message("Communication breakdown on write\n");
        exit(15);
    }
}

static void GSendInt(int val)
{
    GWrite(&val, sizeof(val));
}

static void GSendStr(const char *buf)
{
    unsigned len = buf ? strlen(buf) + 1 : 0;
    GWrite(&len, sizeof(len));
    GWrite(buf, len);
}

static void GSendArr(int len, const char *buf)
{
    GWrite(&len, sizeof(len));
    GWrite(buf, len);
}

static int GRecvInt(void)
{
    int val;

    GRead(&val, sizeof(val));
    return val;
}

static char *GRecvStr(void)
{
    unsigned len;
    char *buf;

    if (!(len = GRecvInt())) {
        return (char *)0;
    }
    if (len > 0x1000 || !(buf = malloc(len))) {
        message("No memory for read buffer\n");
        exit(15);
    }
    GRead(buf, len);
    buf[len - 1] = 0; /* we're setuid ... don't trust "them" */
    return buf;
}

static char *GRecvArr(void)
{
    unsigned len;
    char *arr;
    unsigned const char *up;

    if (!(len = (unsigned)GRecvInt())) {
        return (char *)0;
    }
    if (len < 4) {
        message("Too short binary authentication data block\n");
        exit(15);
    }
    if (len > 0x10000 || !(arr = malloc(len))) {
        message("No memory for read buffer\n");
        exit(15);
    }
    GRead(arr, len);
    up = (unsigned const char *)arr;
    if (len != (unsigned)(up[3] | (up[2] << 8) | (up[1] << 16) | (up[0] << 24))) {
        message("Mismatched binary authentication data block size\n");
        exit(15);
    }
    return arr;
}

static char *conv_server(ConvRequest what, const char *prompt)
{
    GSendInt(what);
    switch (what) {
    case ConvGetBinary: {
        unsigned const char *up = (unsigned const char *)prompt;
        int len = up[3] | (up[2] << 8) | (up[1] << 16) | (up[0] << 24);
        GSendArr(len, prompt);
        return GRecvArr();
    }
    case ConvGetNormal:
    case ConvGetHidden: {
        char *msg;
        GSendStr(prompt);
        msg = GRecvStr();
        if (msg && (GRecvInt() & IsPassword) && !*msg) {
            nullpass = 1;
        }
        return msg;
    }
    case ConvPutAuthSucceeded:
    case ConvPutAuthFailed:
    case ConvPutAuthError:
    case ConvPutAuthAbort:
    case ConvPutReadyForAuthentication:
        return 0;
    case ConvPutInfo:
    case ConvPutError:
    default:
        GSendStr(prompt);
        return 0;
    }
}

void message(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

static void ATTR_NORETURN usage(int exitval)
{
    message(
        "usage: kcheckpass {-h|[-c caller] [-m method] -S handle}\n"
        "  options:\n"
        "    -h           this help message\n"
        "    -S handle    operate in binary server mode on file descriptor handle\n"
        "    -m method    use the specified authentication method (default: \"classic\")\n"
        "  exit codes:\n"
        "    0 success\n"
        "    1 invalid password\n"
        "    2 cannot read password database\n"
        "    Anything else tells you something's badly hosed.\n");
    exit(exitval);
}

int main(int argc, char **argv)
{
    const char *method = "classic";
    const char *username = 0;
    char *p;
    struct passwd *pw;
    int c, nfd;
    uid_t uid;
    AuthReturn ret;
    sigset_t signalMask;
#if HAVE_SIGNALFD_H
    int signalFd;
    struct signalfd_siginfo fdsi;
    ssize_t sigReadSize;
#endif
#if HAVE_EVENT_H
    /* Event Queue */
    int keventQueue;
    /* Listen for two events: SIGUSR1 and SIGUSR2 */
    struct kevent keventEvent[2];
    int keventData;
#endif
    pid_t parentPid;

    parentPid = getppid();

    // disable ptrace on kcheckpass
#if HAVE_PR_SET_DUMPABLE
    prctl(PR_SET_DUMPABLE, 0);
#endif
#if HAVE_PROC_TRACE_CTL
    int mode = PROC_TRACE_CTL_DISABLE;
    procctl(P_PID, getpid(), PROC_TRACE_CTL, &mode);
#endif

    // prevent becoming an orphan while waiting for SIGUSR2
#if HAVE_PR_SET_DUMPABLE
    prctl(PR_SET_PDEATHSIG, SIGUSR2);
#endif

    /* Make sure stdout/stderr are open */
    for (c = 1; c <= 2; c++) {
        if (fcntl(c, F_GETFL) == -1) {
            if ((nfd = open("/dev/null", O_WRONLY)) < 0) {
                message("cannot open /dev/null: %s\n", strerror(errno));
                exit(10);
            }
            if (c != nfd) {
                dup2(nfd, c);
                close(nfd);
            }
        }
    }

    havetty = isatty(0);

    while ((c = getopt(argc, argv, "hm:S:")) != -1) {
        switch (c) {
        case 'h':
            usage(0);
            break;
        case 'm':
            method = optarg;
            break;
        case 'S':
            sfd = atoi(optarg);
            break;
        default:
            message("Command line option parsing error\n");
            usage(10);
        }
    }

    if (sfd == -1) {
        message("Only binary protocol supported\n");
        return AuthError;
    }

    uid = getuid();
    if (!(p = getenv("LOGNAME")) || !(pw = getpwnam(p)) || pw->pw_uid != uid) {
        if (!(p = getenv("USER")) || !(pw = getpwnam(p)) || pw->pw_uid != uid) {
            if (!(pw = getpwuid(uid))) {
                message("Cannot determinate current user\n");
                return AuthError;
            }
        }
    }
    if (!(username = strdup(pw->pw_name))) {
        message("Out of memory\n");
        return AuthError;
    }

    // setup signals
    sigemptyset(&signalMask);
    sigaddset(&signalMask, SIGUSR1);
    sigaddset(&signalMask, SIGUSR2);
    // block them
    if (sigprocmask(SIG_BLOCK, &signalMask, NULL) == -1) {
        message("Block signal failed\n");
        conv_server(ConvPutAuthError, 0);
        return 1;
    }
#if HAVE_SIGNALFD_H
    signalFd = signalfd(-1, &signalMask, SFD_CLOEXEC);
    if (signalFd == -1) {
        message("Signal fd failed\n");
        conv_server(ConvPutAuthError, 0);
        return 1;
    }
#endif
#if HAVE_EVENT_H
    /* Setup the kequeu */
    keventQueue = kqueue();
    if (keventQueue == -1) {
        message("Failed to create kqueue for SIGUSR1\n");
        conv_server(ConvPutAuthError, 0);
        return 1;
    }
    /* Setup the events */
    EV_SET(&keventEvent[0], SIGUSR1, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
    EV_SET(&keventEvent[1], SIGUSR2, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
    int setupResult = kevent(keventQueue, keventEvent, 2, NULL, 0, NULL);
    if (setupResult == -1) {
        message("Failed to attach event to the kqueue\n");
        conv_server(ConvPutAuthError, 0);
        return 1;
    }
    if (keventEvent[0].flags & EV_ERROR) {
        message("Error in kevent for SIGUSR1: %s\n", strerror(keventEvent[0].data));
        conv_server(ConvPutAuthError, 0);
        return 1;
    }
    if (keventEvent[1].flags & EV_ERROR) {
        message("Error in kevent for SIGUSR2: %s\n", strerror(keventEvent[1].data));
        conv_server(ConvPutAuthError, 0);
        return 1;
    }

    /* signal_info for sigwaitinfo() */
    siginfo_t signalInfo;

#endif
    // now lets block on the fd
    for (;;) {
        conv_server(ConvPutReadyForAuthentication, 0);
#if HAVE_SIGNALFD_H
        sigReadSize = read(signalFd, &fdsi, sizeof(struct signalfd_siginfo));
        if (sigReadSize != sizeof(struct signalfd_siginfo)) {
            message("Read wrong size\n");
            return 1;
        }
        if (fdsi.ssi_signo == SIGUSR1) {
            if (fdsi.ssi_pid != parentPid) {
                message("signal from wrong process\n");
                continue;
            }
#endif
#if HAVE_EVENT_H
            keventData = kevent(keventQueue, NULL, 0, keventEvent, 1, NULL);
            if (keventData == -1) {
                /* Let's figure this out in the future, shall we */
                message("kevent() failed with %d\n", errno);
                return 1;
            } else if (keventData == 0) {
                /* Do we need to handle timeouts? */
                message("kevent timeout\n");
                continue;
            }
            // We know we got a SIGUSR1 or SIGUSR2, so fetch it via sigwaitinfo()
            // (otherwise, we could have used sigtimedwait() )
            int signalReturn = sigwaitinfo(&signalMask, &signalInfo);
            if (signalReturn < 0) {
                if (errno == EINTR) {
                    message("sigawaitinfo() interrupted by unblocked caught signal");
                    continue;
                } else if (errno == EAGAIN) {
                    /* This should not happen, as kevent notified us about such a signal */
                    message("no signal of type USR1 or USR2 pending.");
                    continue;
                } else {
                    message("Unhandled error in sigwaitinfo()");
                    conv_server(ConvPutAuthError, 0);
                    return 1;
                }
            }
            if (signalReturn == SIGUSR1) {
                if (signalInfo.si_pid != parentPid) {
                    message("signal from wrong process\n");
                    continue;
                }
#endif
                /* Now do the fandango */
                ret = Authenticate(method, username, conv_server);

                if (ret == AuthBad) {
                    message("Authentication failure\n");
                    if (!nullpass) {
                        openlog("kcheckpass", LOG_PID, LOG_AUTH);
                        syslog(LOG_NOTICE, "Authentication failure for %s (invoked by uid %d)", username, uid);
                    }
                }
                switch (ret) {
                case AuthOk:
                    conv_server(ConvPutAuthSucceeded, 0);
                    break;
                case AuthBad:
                    conv_server(ConvPutAuthFailed, 0);
                    break;
                case AuthError:
                    conv_server(ConvPutAuthError, 0);
                    break;
                case AuthAbort:
                    conv_server(ConvPutAuthAbort, 0);
                default:
                    break;
                }
                if (uid != geteuid()) {
                    // we don't support multiple auth for setuid kcheckpass
                    break;
                }
#if HAVE_SIGNALFD_H
            } else if (fdsi.ssi_signo == SIGUSR2) {
                if (fdsi.ssi_pid != parentPid) {
                    message("signal from wrong process\n");
                    continue;
                }
                break;
#endif
#if HAVE_EVENT_H
            } else if (signalReturn == SIGUSR2) {
                if (signalInfo.si_pid != parentPid) {
                    message("signal from wrong process\n");
                    continue;
                }
                break;
#endif
            } else {
                message("unexpected signal\n");
            }
        }

        return 0;
    }

    void dispose(char *str)
    {
        memset(str, 0, strlen(str));
        free(str);
    }

    /*****************************************************************
      The real authentication methods are in separate source files.
      Look in checkpass_*.c
    *****************************************************************/
