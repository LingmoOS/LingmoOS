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

#include "connection.h"
#include "logger.h"

#include <sys/socket.h>
#include <sys/un.h>       /* for getsockopt */
#include <sys/stat.h>     /* for chmod */
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <stdexcept>
#include <sys/syslog.h>

BEGIN_NAMESPACE
Connection::Connection(int socketFd, bool testMode) :
        m_testMode(testMode),
        m_fd(-1),
        m_curSocket(socketFd),
        m_fileName(""),
        m_argc(0),
        m_argv(NULL),
        m_priority(0),
        m_delay(0),
        m_sendPid(false),
        m_gid(0),
        m_uid(0)
{
    m_io[0] = -1;
    m_io[1] = -1;
    m_io[2] = -1;

    if (!m_testMode && m_curSocket == -1)
        throw std::runtime_error("Connection: Socket isn't initialized!\n");
}

Connection::~Connection()
{
    close();

    for (int i = 0; i < IO_DESCRIPTOR_COUNT; i++)
    {
        if (m_io[i] != -1)
        {
            ::close(m_io[i]);
            m_io[i] = -1;
        }
    }
}


int Connection::getFd() const
{
    return m_fd;
}

bool Connection::accept(AppData* appData)
{
    if (!m_testMode)
    {
        m_fd = ::accept(m_curSocket, NULL, NULL);

        if (m_fd < 0)
        {
            Logger::logError("Connection: Failed to accept a connection: %s\n", strerror(errno));
            return false;
        }
    }

    return true;
}

bool Connection::connected() const
{
    return m_fd > -1;
}

void Connection::close()
{
    if (m_fd != -1)
    {
        if (!m_testMode)
        {
            ::close(m_fd);
        }

        m_fd = -1;
    }
}

bool Connection::sendMsg(uint32_t msg)
{
    if (!m_testMode)
    {
        Logger::logDebug("Connection: %s: %08x", __FUNCTION__, msg);
        return write(m_fd, &msg, sizeof(msg)) != -1;
    }
    else
    {
        return true;
    }
}

bool Connection::recvMsg(uint32_t *msg)
{
    if (!m_testMode)
    {
        uint32_t buf = 0;
        int len = sizeof(buf);
        ssize_t ret = read(m_fd, &buf, len);

        if (ret < len)
        {
            Logger::logError("Connection: can't read data from connecton in %s", __FUNCTION__);
            *msg = 0;
        }
        else
        {
            Logger::logDebug("Connection: %s: %08x", __FUNCTION__, *msg);
            *msg = buf;
        }

        return ret != -1;
    }
    else
    {
        return true;
    }
}

const char * Connection::recvStr()
{
    if (!m_testMode)
    {
        // Get the size.
        uint32_t size = 0;

        const uint32_t STR_LEN_MAX = 65536;
        bool res = recvMsg(&size);
        if (!res || size == 0 || size > STR_LEN_MAX)
        {
            Logger::logError("Connection: string receiving failed in %s, string length is %d", __FUNCTION__, size);
            return NULL;
        }

        char * str = new char[size];
        if (!str)
        {
            Logger::logError("Connection: mallocing in %s", __FUNCTION__);
            return NULL;
        }

        // Get the string.
        uint32_t ret = read(m_fd, str, size);
        if (ret < size)
        {
            Logger::logError("Connection: getting string, got %u of %u bytes", ret, size);
            delete [] str;
            return NULL;
        }

        str[size - 1] = '\0';
        Logger::logDebug("Connection: %s: '%s'", __FUNCTION__, str);

        return str;
    }
    else
    {
        return NULL;
    }
}

bool Connection::sendPid(pid_t pid)
{
    sendMsg(INVOKER_MSG_PID);
    sendMsg(pid);

    return true;
}

bool Connection::sendExitValue(int value)
{
    sendMsg(INVOKER_MSG_EXIT);
    sendMsg(value);

    return true;
}

uint32_t Connection::receiveMagic()
{
    uint32_t magic = 0;

    // Receive the magic.
    recvMsg(&magic);

    if ((magic & INVOKER_MSG_MASK) == INVOKER_MSG_MAGIC)
    {
        if (!((magic & INVOKER_MSG_MAGIC_VERSION_MASK) == INVOKER_MSG_MAGIC_VERSION))
        {
            Logger::logError("Connection: receiving bad magic version (%08x)\n", magic);
            return -1;
        }
    }
    m_sendPid  = magic & INVOKER_MSG_MAGIC_OPTION_WAIT;

    return magic & INVOKER_MSG_MAGIC_OPTION_MASK;
}

string Connection::receiveAppName()
{
    uint32_t msg = 0;

    // Get the action.
    recvMsg(&msg);
    if (msg != INVOKER_MSG_NAME)
    {
        Logger::logError("Connection: receiving invalid action (%08x)", msg);
        return string();
    }

    const char* name = recvStr();
    if (!name)
    {
        Logger::logError("Connection: receiving application name");
        return string();
    }

    string appName(name);
    delete [] name;
    return appName;
}

bool Connection::receiveExec()
{
    const char* filename = recvStr();
    if (!filename)
        return false;

    m_fileName = filename;
    delete [] filename;
    return true;
}

bool Connection::receivePriority()
{
    recvMsg(&m_priority);
    return true;
}

bool Connection::receiveDelay()
{
    recvMsg(&m_delay);
    return true;
}

bool Connection::receiveIDs()
{
    recvMsg(&m_uid);
    recvMsg(&m_gid);
    return true;
}

bool Connection::receiveArgs()
{
    // Get argc
    recvMsg(&m_argc);
    const uint32_t ARG_MAX = 1024;
    if (m_argc > 0 && m_argc < ARG_MAX)
    {
        // Reserve memory for argv
        m_argv = new const char * [m_argc];
        if (!m_argv)
        {
            Logger::logError("Connection: reserving memory for argv");
            return false;
        }

        // Get argv
        for (uint i = 0; i < m_argc; i++)
        {
            m_argv[i] = recvStr();
            if (!m_argv[i])
            {
                Logger::logError("Connection: receiving argv[%i]", i);
                return false;
            }
        }
    }
    else
    {
        Logger::logError("Connection: invalid number of parameters %d", m_argc);
        return false;
    }

    return true;
}

// coverity[ +tainted_string_sanitize_content : arg-0 ]
bool putenv_sanitize(const char * s)
{
    return static_cast<bool>(strchr(s, '='));
}

// coverity[ +free : arg-0 ]
int putenv_wrapper(char * var)
{
    return putenv(var);
}

bool Connection::receiveEnv()
{
    // Have some "reasonable" limit for environment variables to protect from
    // malicious data
    const uint32_t MAX_VARS = 1024;

    // Get number of environment variables.
    uint32_t n_vars = 0;
    recvMsg(&n_vars);
    if (n_vars > 0 && n_vars < MAX_VARS)
    {
        // Get environment variables
        for (uint32_t i = 0; i < n_vars; i++)
        {
            const char * var = recvStr();
            if (var == NULL)
            {
                Logger::logError("Connection: receiving environ[%i]", i);
                return false;
            }

            // In case of error, just warn and try to continue, as the other side is
            // going to keep sending the reset of the message.
            // String pointed to by var shall become part of the environment, so altering
            // the string shall change the environment, don't free it
            if (putenv_sanitize(var))
            {
                if (putenv_wrapper(const_cast<char *>(var)) != 0)
                {
                    Logger::logWarning("Connection: putenv failed");
                }
            }
            else
            {
                delete [] var;
                var = NULL;
                Logger::logWarning("Connection: invalid environment data");
            }
        }
    }
    else
    {
        Logger::logError("Connection: invalid environment variable count %d", n_vars);
        return false;
    }

    return true;
}

bool Connection::receiveIO()
{
    int dummy = 0;

    struct iovec iov;
    iov.iov_base = &dummy;
    iov.iov_len = 1;

    char buf[CMSG_SPACE(sizeof(m_io))];
    
    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov        = &iov;
    msg.msg_iovlen     = 1;
    msg.msg_control    = buf;
    msg.msg_controllen = sizeof(buf);

    struct cmsghdr *cmsg;
    cmsg             = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_len   = CMSG_LEN(sizeof(m_io));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type  = SCM_RIGHTS;

    memcpy(CMSG_DATA(cmsg), m_io, sizeof(m_io));

    if (recvmsg(m_fd, &msg, 0) < 0)
    {
        Logger::logWarning("Connection: recvmsg failed in invoked_get_io: %s", strerror(errno));
        return false;
    }

    if (msg.msg_flags)
    {
        Logger::logWarning("Connection: unexpected msg flags in invoked_get_io");
        return false;
    }

    cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg == NULL || cmsg->cmsg_len != CMSG_LEN(sizeof(m_io)) ||
        cmsg->cmsg_level != SOL_SOCKET || cmsg->cmsg_type != SCM_RIGHTS)
    {
        Logger::logWarning("Connection: invalid cmsg in invoked_get_io");
        return false;
    }

    memcpy(m_io, CMSG_DATA(cmsg), sizeof(m_io));

    return true;
}

bool Connection::receiveActions()
{
    Logger::logDebug("Connection: enter: %s", __FUNCTION__);

    while (1)
    {
        uint32_t action = 0;

        // Get the action.
        recvMsg(&action);

        switch (action)
        {
        case INVOKER_MSG_EXEC:
            receiveExec();
            break;

        case INVOKER_MSG_ARGS:
            receiveArgs();
            break;

        case INVOKER_MSG_ENV:
            // Clean-up all the env variables
            clearenv();
            receiveEnv();
            break;

        case INVOKER_MSG_PRIO:
            receivePriority();
            break;

        case INVOKER_MSG_DELAY:
            receiveDelay();
            break;

        case INVOKER_MSG_IO:
            receiveIO();
            break;

        case INVOKER_MSG_IDS:
            receiveIDs();
            break;

        case INVOKER_MSG_SPLASH:
            Logger::logError("Connection: received a now-unsupported MSG_SPLASH\n");
            return false;

        case INVOKER_MSG_LANDSCAPE_SPLASH:
            Logger::logError("Connection: received a now-unsupported MSG_LANDSCAPE_SPLASH\n");
            return false;

        case INVOKER_MSG_END:
            sendMsg(INVOKER_MSG_ACK);

            if (m_sendPid)
                sendPid(getpid());

            return true;

        default:
            Logger::logError("Connection: received invalid action (%08x)\n", action);
            return false;
        }
    }
}

bool Connection::receiveApplicationData(AppData* appData)
{
    // Read magic number
    appData->setOptions(receiveMagic());
    if (appData->options() == -1)
    {
        Logger::logError("Connection: receiving magic failed\n");
        return false;
    }

    // Read application name
    appData->setAppName(receiveAppName());
    if (appData->appName().empty())
    {
        Logger::logError("Connection: receiving application name failed\n");
        return false;
    }

    // Read application parameters
    if (receiveActions())
    {
        appData->setFileName(m_fileName);
        appData->setPriority(m_priority);
        appData->setDelay(m_delay);
        appData->setArgc(m_argc);
        appData->setArgv(m_argv);
        appData->setIODescriptors(vector<int>(m_io, m_io + IO_DESCRIPTOR_COUNT));
        appData->setIDs(m_uid, m_gid);
    }
    else
    {
        Logger::logError("Connection: receiving application parameters failed\n");
        return false;
    }

    return true;
}

bool Connection::isReportAppExitStatusNeeded() const
{
    return m_sendPid;
}

pid_t Connection::peerPid()
{
    struct ucred cr;

    socklen_t len = sizeof(struct ucred);
    if (getsockopt(m_fd, SOL_SOCKET, SO_PEERCRED, &cr, &len) < 0)
    {
        Logger::logError("Connection: can't get peer's pid: %s\n", strerror(errno));
        return 0;
    }
    return cr.pid;

}
END_NAMESPACE