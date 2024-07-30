/* vi: ts=8 sts=4 sw=4
 *
    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <g.t.jansen@stud.tue.nl>

    secure.cpp: Peer credentials for a UNIX socket.
*/

#include "secure.h"

#include <config-kdesud.h>
#include <ksud_debug.h>

#include <cerrno>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

// FIXME: This is just here to make it compile (since ksock* was removed from kdelibs).
// It would be better to fix it more globally. (Caleb Tennis)
typedef unsigned ksocklen_t;

/**
 * Under Linux, Socket_security is supported.
 */

#if HAVE_GETPEEREID

SocketSecurity::SocketSecurity(int sockfd)
    : pid(-1)
    , gid(-1)
    , uid(-1)
{
    uid_t euid;
    gid_t egid;
    if (getpeereid(sockfd, &euid, &egid) == 0) {
        uid = euid;
        gid = egid;
        pid = -1;
    }
}

#elif HAVE_GETPEERUCRED

#include <ucred.h>

SocketSecurity::SocketSecurity(int sockfd)
    : pid(-1)
    , gid(-1)
    , uid(-1)
{
    ucred_t *ucred = 0;

    if (getpeerucred(sockfd, &ucred) == 0) {
        uid = ucred_geteuid(ucred);
        gid = ucred_getrgid(ucred);
        pid = -1;
        ucred_free(ucred);
    }
}

#elif defined(SO_PEERCRED)

SocketSecurity::SocketSecurity(int sockfd)
    : pid(-1)
    , gid(-1)
    , uid(-1)
{
    ucred cred;
    ksocklen_t len = sizeof(struct ucred);
    if (getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &cred, &len) < 0) {
        qCCritical(KSUD_LOG) << "getsockopt(SO_PEERCRED) " << strerror(errno);
        return;
    }
    pid = cred.pid;
    gid = cred.gid;
    uid = cred.uid;
}

#else
#ifdef __GNUC__
#warning SocketSecurity support for your platform not implemented/available!
#endif
/**
 * The default version does nothing.
 */

SocketSecurity::SocketSecurity(int sockfd)
    : pid(-1)
    , gid(-1)
    , uid(-1)
{
    static bool warned_him = false;

    if (!warned_him) {
        qCWarning(KSUD_LOG) << "Using void socket security. Please add support for your";
        qCWarning(KSUD_LOG) << "platform to src/kdesud/secure.cpp";
        warned_him = true;
    }

    // This passes the test made in handler.cpp
    uid = getuid();
}

#endif
