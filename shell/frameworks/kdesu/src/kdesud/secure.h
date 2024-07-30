/* vi: ts=8 sts=4 sw=4

    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>
*/

#ifndef __Secure_h_included__
#define __Secure_h_included__

#include <sys/socket.h>
#include <sys/types.h>

/**
 * The Socket_security class authenticates the peer for you. It provides
 * the process-id, user-id and group-id plus the MD5 sum of the connected
 * binary.
 */

class SocketSecurity
{
public:
    explicit SocketSecurity(int fd);

    /** Returns the peer's process-id. */
    int peerPid() const
    {
        return pid;
    }

    /** Returns the peer's user-id */
    int peerUid() const
    {
        return uid;
    }

    /** Returns the peer's group-id */
    int peerGid() const
    {
        return gid;
    }

private:
    int pid;
    int gid;
    int uid;
};

#endif
