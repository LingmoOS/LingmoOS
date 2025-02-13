/***************************************************************************
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (c) 2012 - 2021 Jolla Ltd.
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

#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include "launcherlib.h"
#include <map>
#include <string>

using std::map;
using std::string;

/*!
 * \class SocketManager
 *
 * SocketManager Manages sockets that are used in the invoker <-> booster
 * communication.
 */
class DECL_EXPORT SocketManager
{
public:
    SocketManager();

    string prepareSocket(const string &socketId) const;

    /*! \brief Initialize a file socket.
     *  \param socketId Path to the socket file.
     */
    void initSocket(const string & socketId);

    /*! \brief Close a file socket.
     *  \param socketId Path to the socket file.
     */
    void closeSocket(const string & socketId);

    //! \brief Close all open sockets.
    void closeAllSockets();

    /*! \brief Return initialized socket.
     *  \param socketId Path to the socket file.
     *  \returns socket fd or -1 on failure.
     */
    int findSocket(const string & socketId);

    //! Return count of currently active sockets
    unsigned int socketCount() const;

    // Type of the internal state
    typedef map<string, int> SocketHash;

    //! Get the state
    SocketHash getState();

    //! Add mapping of socketId to fd
    void addMapping(const string & socketId, int fd);

    /*!
     * Root path for booster sockets
     */
    string socketRootPath() const;

private:

    SocketHash m_socketHash;

    //! Root path for booster sockets
    string m_socketRootPath;

};

#endif // SOCKETMANAGER_H
