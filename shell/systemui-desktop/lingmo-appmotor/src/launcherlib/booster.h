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

#ifndef BOOSTER_H
#define BOOSTER_H

#include "launcherlib.h"

#include <cstdlib>
#include <string>

using std::string;

#include "appdata.h"

class Connection;
class SocketManager;
class SingleInstance;

/*!
 *  \class Booster
 *  \brief Abstract base class for all boosters (Qt-booster, M-booster and so on..)
 *
 *  Booster is a class that is used to initialize certain resources in libraries
 *  common to all applications of a type.
 *
 *  Booster also communicates with the invoker process and handles the actual
 *  jump to the main() -method of the application to be launched.
 *
 *  Booster instance dies with the launched application and a new one must be created
 *  in advance so as to launch a new application.
 */
class DECL_EXPORT Booster
{
public:

    //! Constructor
    Booster();

    //! Destructor
    virtual ~Booster();

    /*!
     * \brief Initializes the booster process.
     * \param initialArgc argc of the parent process.
     * \param initialArgv argv of the parent process.
     * \param boosterLauncherSocket socket connection to the parent process.
     * \param socketFd socket used to get commands from the invoker.
     * \param singleInstance Pointer to a valid SingleInstance object.
     * \param bootMode Booster-specific preloads are not executed if true.
     */
    virtual void initialize(int initialArgc, char ** initialArgv, int boosterLauncherSocket,
                            int socketFd, SingleInstance * singleInstance,
                            bool bootMode);

    /*!
     * \brief Run the application to be invoked.
     * By default, this method causes the application binary to be loaded
     * using dlopen(). Program execution jumps to the address of
     * "main()" found in the newly loaded library. The Booster process
     * exits with corresponding exit-code after the execution of
     * main() has finished. run() returns the return value of the main()
     * function.
     *
     * \param socketManager Pointer to the SocketManager so that
     * we can close all needless sockets in the application process.
     */
    virtual int run(SocketManager * socketManager);

    /*!
     * \brief Rename process.
     * This method overrides the argument data starting from initialArgv[0].
     * This is needed so as to get the process name and arguments displayed
     * correctly e.g. in the listing by 'ps'. initialArgv[1] may provide an
     * empty dummy space to be used. It is assumed, that the arguments are
     * located continuosly in memory and this is how it's done in glibc.
     *
     * \param initialArgc Number of the arguments of the launcher process.
     * \param initialArgv Address of the argument array of the launcher process.
     */
    void renameProcess(int parentArgc, char** parentArgv,
                       int sourceArgc, const char** sourceArgv);

    /*!
     * \brief Return unique booster name
     *
     * The booster name is used as a parameter to invoker to launch processes
     * with the given booster, and is used as the name for the booster socket.
     *
     * \return A (unique) string representing the type of the Booster.
     */
    virtual const string & boosterType() const = 0;

    const string &boostedApplication() const;
    void setBoostedApplication(const string &application);

    const string socketId() const;

    //! Get invoker's pid
    pid_t invokersPid();

    //! Get the connection object
    Connection* connection() const;

    //! Set connection object. Booster takes the ownership.
    void setConnection(Connection * connection);

    //! Get application data object
    AppData* appData() const;

    //! Return true, if in boot mode.
    bool bootMode() const;

protected:

    /*!
     * Set process environment (UID, GID..) before launch.
     * Re-implement if needed. This is automatically called by
     * launchProcess().
     */
    virtual void setEnvironmentBeforeLaunch();

    /*! Load the library and jump to main.
     * Re-implement if needed.
     */
    virtual int launchProcess();

    /*!
     * \brief Preload libraries / initialize cache etc.
     * Called from initialize if not in the boot mode.
     * Re-implement in the custom Booster.
     */
    virtual bool preload() = 0;

    /*!
     * \brief Wait for connection from invoker and read the input.
     * This method accepts a socket connection from the invoker
     * and reads the data of an application to be launched.
     *
     * \param socketFd Fd of the UNIX socket file.
     * \return true on success
     */
    virtual bool receiveDataFromInvoker(int socketFd);

    /*! This method is called just before call boosted application's
     *  main function. Empty by default but some booster specific
     *  initializations can be done here.
     *  Re-implement if needed.
     */
    virtual void preinit() {};

    //! Set nice value and store the old priority. Return true on success.
    bool pushPriority(int nice);

    //! Restore the old priority stored by the previous successful setPriority().
    bool popPriority();

    //! Sets the socket fd used in the communication between
    //! the booster and launcher.
    void setBoosterLauncherSocket(int boosterLauncherSocket);

    //! Returns the socket fd between the booster and launcher.
    int boosterLauncherSocket() const;

    //! Reset out-of-memory killer adjustment
    void resetOomAdj();

    //! Data structure representing the application to be invoked
    AppData* m_appData;

private:

    //! Disable copy-constructor
    Booster(const Booster & r);

    //! Disable assignment operator
    Booster & operator= (const Booster & r);

    //! Send data to the parent process (invokers pid, respwan delay)
    //! and signal that a new booster can be created.
    void sendDataToParent();

    //! Helper method: load the library and find out address for "main".
    void* loadMain();

    //! Helper method: returns application name for to use for locking etc.
    std::string getFinalName(const std::string &name);

    //! Socket connection to invoker
    Connection* m_connection;

    //! Process priority before pushPriority() is called
    int m_oldPriority;

    //! True if m_oldPriority is a valid value so that
    //! it can be restored later.
    bool m_oldPriorityOk;

    //! Socket used to tell the parent that a new booster is needed
    int m_boosterLauncherSocket;

    //! Original space available for arguments
    int m_spaceAvailable;

    //! Application name when applicable, or "default"
    string m_boostedApplication;

    //! True, if being run in boot mode.
    bool m_bootMode;

#ifdef UNIT_TEST
    friend class Ut_Booster;
#endif
};

#endif // BOOSTER_H
