/***************************************************************************
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (c) 2013 - 2021 Jolla Ltd.
** Copyright (c) 2021 Open Mobile Platform LLC.
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

SocketManager::SocketManager()
{
    const char *runtimeDir = getenv("XDG_RUNTIME_DIR");
    if (!runtimeDir || !*runtimeDir)
        runtimeDir = "/tmp";

    m_socketRootPath = runtimeDir;
    m_socketRootPath += "/mapplauncherd";

    if (mkdir(m_socketRootPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
        if (errno != EEXIST) {
            Logger::logError("Daemon: Cannot create socket root directory %s: %s\n",
                             m_socketRootPath.c_str(), strerror(errno));
        }
    }
}

static string extractTail(string &work)
{
    string tok;
    string::size_type n;
    if ((n = work.rfind('/')) == string::npos) {
        tok = work;
        work.clear();
    } else {
        tok = work.substr(n + 1);
        work.erase(n);
    }
    return tok;
}

string SocketManager::prepareSocket(const string &socketId) const
{
    string socketPath;

    /* Extract APP/TYPE/SOCKET */
    string work(socketId);
    string socketFile(extractTail(work));
    string typeId(extractTail(work));
    string appId(extractTail(work));

    if (!work.empty() || socketFile.empty()) {
        Logger::logError("Daemon: Invalid socketId: %s\n", socketId.c_str());
        return socketPath;
    }

    /* Construct socket path and subdirectories */
    work = m_socketRootPath;

    if (!appId.empty()) {
        work += '/';
        work += appId;
        if (mkdir(work.c_str(), 0750) == -1 && errno != EEXIST) {
            Logger::logError("Daemon: Cannot create socket app directory %s: %s\n",
                             work.c_str(), strerror(errno));
            return socketPath;
        }
    }

    if (!typeId.empty()) {
        work += '/';
        work += typeId;
        if (mkdir(work.c_str(), 0750) == -1 && errno != EEXIST) {
            Logger::logError("Daemon: Cannot create socket type directory %s: %s\n",
                             work.c_str(), strerror(errno));
            return socketPath;
        }
    }

    work += '/';
    work += socketFile;
    if (unlink(work.c_str()) == -1 && errno != ENOENT) {
        Logger::logError("Daemon: Cannot remove stale socket %s: %s\n",
                         work.c_str(), strerror(errno));
        return socketPath;
    }

    /* Success */
    socketPath = work;
    return socketPath;
}

void SocketManager::initSocket(const string & socketId)
{
    // Initialize a socket at socketId if one already doesn't
    // exist for that id / path.
    if (m_socketHash.find(socketId) == m_socketHash.end())
    {
        string socketPath = prepareSocket(socketId);
        if (socketPath.empty()) {
            string msg;
            msg += "SocketManager: Failed to prepare socketId ";
            msg += socketId;
            throw std::runtime_error(msg);
        }

        Logger::logDebug("SocketManager: Initing socket at '%s'..", socketPath.c_str());

        // Initialize the socket struct
        struct sockaddr_un sun;
        memset(&sun, 0, sizeof sun);
        sun.sun_family = AF_UNIX;

        size_t maxSize = sizeof(sun.sun_path);
        int length = snprintf(sun.sun_path, maxSize, "%s", socketPath.c_str());
        if (length <= 0 || (size_t)length >= maxSize) {
            string msg;
            msg += "SocketManager: Invalid socket path ";
            msg += socketPath;
            throw std::runtime_error(msg);
        }

        // Create a new local socket
        int socketFd = socket(PF_UNIX, SOCK_STREAM, 0);
        if (socketFd < 0)
            throw std::runtime_error("SocketManager: Failed to open socket\n");

        // Bind the socket
        if (bind(socketFd, (struct sockaddr*) &sun, sizeof(sun)) < 0)
        {
            string msg;
            msg += "SocketManager: Failed to bind socket to ";
            msg += socketPath;
            throw std::runtime_error(msg);
        }

        // Listen to the socket
        if (listen(socketFd, 10) < 0)
        {
            string msg;
            msg += "SocketManager: Failed to listen to socket ";
            msg += socketPath;
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
    return m_socketRootPath + '/';
}

