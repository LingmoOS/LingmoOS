/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include "socketmanager.h"
#include "logger.h"

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <errno.h>
#include <sstream>

BEGIN_NAMESPACE
SocketManager::SocketManager()
{
    const char *runtimeDir = getenv("XDG_RUNTIME_DIR");
    if (!runtimeDir || !*runtimeDir)
        runtimeDir = "/tmp/";

    m_socketRootPath = runtimeDir;
    m_socketRootPath += "/mapplauncherd";

    if (mkdir(m_socketRootPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
        if (errno != EEXIST) {
            Logger::logError("Daemon: Cannot create socket root directory %s: %s\n",
                             m_socketRootPath.c_str(), strerror(errno));
        }
    }

    m_socketRootPath += '/';
}

void SocketManager::initSocket(const string & socketId)
{
    string socketPath = m_socketRootPath + socketId;

    // Initialize a socket at socketId if one already doesn't
    // exist for that id / path.
    if (m_socketHash.find(socketId) == m_socketHash.end())
    {
        Logger::logDebug("SocketManager: Initing socket at '%s'..", socketPath.c_str());

        // Create a new local socket
        int socketFd = socket(PF_UNIX, SOCK_STREAM, 0);
        if (socketFd < 0)
            throw std::runtime_error("SocketManager: Failed to open socket\n");

        // TODO: Error if socketPath >= maxLen. Also unlink() here may
        // try to remove a different file than is passed to sun.sa_data.

        // Remove the previous socket file
        struct stat sb;
        stat(socketPath.c_str(), &sb);
        if (S_ISSOCK(sb.st_mode))
        {
            // coverity[toctou]
            if (unlink(socketPath.c_str()) == -1)
            {
                std::string msg("SocketManager: Failed to unlink existing socket file '");
                msg += socketPath + "': " + strerror(errno);
                Logger::logWarning(msg.c_str());
            }
        }

        // Initialize the socket struct
        struct sockaddr_un sun;
        sun.sun_family = AF_UNIX;
        int maxLen = sizeof(sun.sun_path) - 1;
        strncpy(sun.sun_path, socketPath.c_str(), maxLen);
        sun.sun_path[maxLen] = '\0';

        // Bind the socket
        if (bind(socketFd, (struct sockaddr*) &sun, sizeof(sun)) < 0)
        {
            std::string msg("SocketManager: Failed to bind to socket (fd=");
            std::stringstream ss;
            ss << socketFd;
            msg += ss.str() + ")";
            throw std::runtime_error(msg);
        }

        // Listen to the socket
        if (listen(socketFd, 10) < 0)
        {
            std::string msg("SocketManager: Failed to listen to socket (fd=");
            std::stringstream ss;
            ss << socketFd;
            msg += ss.str() + ")";
            throw std::runtime_error(msg);
        }

        // Set permissions
        chmod(socketPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);

        // Store path <-> file descriptor mapping
        m_socketHash[socketId] = socketFd;
    }
}

void SocketManager::closeSocket(const string & socketId)
{
    SocketHash::iterator it(m_socketHash.find(socketId));

    if (it != m_socketHash.end())
    {
        ::close(it->second);
        m_socketHash.erase(it);
    }
}

void SocketManager::closeAllSockets()
{
    SocketHash::iterator it;
    for (it = m_socketHash.begin(); it != m_socketHash.end(); ++it)
    {
        if (it->second > 0)
        {
            ::close(it->second);
        }
    }

    m_socketHash.clear();
}

int SocketManager::findSocket(const string & socketId)
{
    SocketHash::iterator i(m_socketHash.find(socketId));
    return i == m_socketHash.end() ? -1 : i->second;
}

unsigned int SocketManager::socketCount() const
{
    return m_socketHash.size();
}

SocketManager::SocketHash SocketManager::getState()
{
    return m_socketHash;
}

void SocketManager::addMapping(const string & socketId, int fd)
{
    m_socketHash[socketId] = fd;
}

string SocketManager::socketRootPath() const
{
    return m_socketRootPath;
}
END_NAMESPACE
