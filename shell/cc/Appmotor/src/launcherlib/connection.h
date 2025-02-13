/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (C) 2012 - 2021 Jolla Ltd.
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

#ifndef CONNECTION_H
#define CONNECTION_H

#include "launcherlib.h"
#include "appdata.h"
#include "protocol.h"

#include <stdint.h>
#include <string>

using std::string;

#define IO_DESCRIPTOR_COUNT 3

/*!
 * \class Connection.
 * \brief Wrapper class for the connection between invoker and launcher.
 *
 * This class wraps up the UNIX file socket connection between the invoker
 * and the launcher daemon. The low-level communication code is mostly taken
 * from the maemo-launcher used in Maemo 5. It might need a re-write.
 */
class DECL_EXPORT Connection
{
public:

    /*! \brief Constructor.
     *  \param socketFd Fd of the UNIX file socket to be used.
     *  \param testMode Bypass all real socket activity to help unit testing.
     */
    explicit Connection(int socketFd, bool testMode = false);

    //! Destructor
    virtual ~Connection();


    /*! \brief Accept connection.
     * Accept a socket connection from the invoker.
     * Stores security credentials of the connected
     * peer to appData, if security is enabled. The credentials
     * in appData must be released by the caller.
     * \return true on success.
     */
    bool accept(AppData* appData);

    //! \brief Close the socket connection.
    void close();

    //! \brief Check connection state.
    bool connected() const;

    //! \brief Get invoker socket file descriptor
    int getFd() const;

    //! \brief Receive application data to appData.
    bool receiveApplicationData(AppData* appData);

    //! \brief Return true if invoker wait for process exit status
    bool isReportAppExitStatusNeeded() const;

    //! \brief Get pid of the process on the other end of socket connection
    pid_t peerPid();

    //! \brief Send application exit value 
    bool sendExitValue(int value);

private:

    /*! \brief Receive actions.
     * This method executes the actual data-receiving loop and terminates
     * after INVOKER_MSG_END is received.
     * \return True on success
     */
    bool receiveActions();

    /*! \brief Receive and return the magic number.
     * \return The magic number received from the invoker.
     */
    uint32_t receiveMagic();

    /*! \brief Receive and return the application name.
     * \return Name string
     */
    string receiveAppName();

    //! Disable copy-constructor
    Connection(const Connection & r);

    //! Disable assignment operator
    Connection & operator= (const Connection & r);

    //! Receive executable name
    bool receiveExec();

    //! Receive arguments
    bool receiveArgs();

    //! Receive environment
    bool receiveEnv();

    //! Receive I/O descriptors
    bool receiveIO();

    //! Receive userId and GroupId
    bool receiveIDs();

    //! Receive priority
    bool receivePriority();

    //! Receive booster respawn delay
    bool receiveDelay();

    //! Send process pid
    bool sendPid(pid_t pid);

    //! Send message to a socket. This is a virtual to help unit testing.
    virtual bool sendMsg(uint32_t msg);

    //! Receive a message from a socket. This is a virtual to help unit testing.
    virtual bool recvMsg(uint32_t *msg);

    //! Receive a string. This is a virtual to help unit testing.
    virtual char *recvStr();

    //! Run in test mode, if true
    bool m_testMode;

    //! Fd of an accepted connection
    int m_fd;

    //! Fd of the UNIX socket file
    int m_curSocket;

    string   m_fileName;
    int      m_argc;
    char   **m_argv;
    int      m_io[IO_DESCRIPTOR_COUNT];
    uint32_t m_priority;
    uint32_t m_delay;
    bool     m_sendPid;
    gid_t    m_gid;
    uid_t    m_uid;


#ifdef UNIT_TEST
    friend class Ut_Connection;
#endif
};

#endif //CONNECTION_H
