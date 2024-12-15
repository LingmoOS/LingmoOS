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

#include <dbus/dbus.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h> 
extern "C" {
    #include "report.h"
}
#include <stdlib.h>

#define DECL_EXPORT extern "C" __attribute__ ((__visibility__("default")))

namespace
{
    int g_lockFd = -1;
    const std::string LOCK_PATH_BASE(std::string(getenv("XDG_RUNTIME_DIR"))+"/single-instance-locks/");
    const std::string LOCK_FILE_NAME("instance.lock");
}

//! Create a path and its components using mask 0777
static bool mkpath(const std::string & path)
{
    for (unsigned int i = 0; i < path.size(); i++)
    {
        if ((i + 1 < path.size() && path[i + 1] == '/' && path[i] != '/') ||
            (i + 1 == path.size()))
        {
            const std::string part = path.substr(0, i + 1);
            if (mkdir(part.c_str(), 0777) != -1)
            {
                // chmod again, because permissions set by mkdir()
                // are modified by umask
                if (chmod(part.c_str(), 0777) == -1)
                {
                    report(report_error, "chmod() failed: %s \n", strerror(errno));
                    return false;
                }
            }
        }
    }

    return true;
}

//! Print help.
static void printHelp()
{
    printf("\nUsage: %s [options] [application]\n"
           "Launch application as a single instance.\n"
           "If given application is already running, its window is raised.\n"
           "Give the full path to the application binary.\n\n"
           "Options:\n"
           "  -h, --help          Print this help message.\n\n"
           "Example: %s /usr/bin/helloworld\n\n",
           PROG_NAME_SINGLE_INSTANCE, PROG_NAME_SINGLE_INSTANCE);
}


// **** Export these functions when used as a library ****
extern "C"
{
    /*!
     * \brief Try to acquire a lock file.
     *
     * Tries to acquire a lock currently at
     * $XDG_RUNTIME_DIR/single-instance-locks/[binaryName]/instance.lock
     *
     * \param binaryName Full path to the binary.
     * \return true if succeeded, false on failure.
     */
    DECL_EXPORT bool lock(const char * binaryName)
    {
        std::string path(LOCK_PATH_BASE + binaryName);
        if (!mkpath(path))
        {
            report(report_error, "Couldn't create dir %s\n", path.c_str());

            return false;
        }

        path += std::string("/") + LOCK_FILE_NAME;

        struct flock fl;

        fl.l_type   = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start  = 0;
        fl.l_len    = 1;

        if((g_lockFd = open(path.c_str(), O_WRONLY | O_CREAT, 0666)) == -1)
        {
            report(report_error, "Couldn't create/open lock file '%s' : %s\n",
                   path.c_str(), strerror(errno));

            return false;
        }

        if(fcntl(g_lockFd, F_SETLK, &fl) == -1)
        {
            close(g_lockFd);
            return false;
        }
        return true;
    }

    //! Close the lock file acquired by lock()
    DECL_EXPORT void unlock()
    {
        if (g_lockFd != -1)
        {
            close(g_lockFd);
            g_lockFd = -1;
        }
    }

    //! Activate existing application 
    DECL_EXPORT bool activateExistingInstance(const char * binaryName)
    {
        DBusError error;

        dbus_error_init(&error);
        DBusConnection *bus = dbus_bus_get(DBUS_BUS_SESSION, &error);
        if (!bus) {
            report(report_error, "Can't get session bus connection");
            goto err;
        }

        DBusMessage *msg;
        DBusMessageIter args;

        msg = dbus_message_new_method_call("org.nemomobile.lipstick",
                                           "/WindowModel",
                                           "local.Lipstick.WindowModel",
                                           "launchProcess");
        if (!msg) {
            report(report_error, "Can't allocate bus message");
            goto err;
        }

        dbus_message_iter_init_append(msg, &args);
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &binaryName)) {
            report(report_error, "Can't allocate bus message");
            goto err;
        }

        if (!dbus_connection_send(bus, msg, NULL)) {
            report(report_error, "Can't send message");
            goto err;
        }

        /* as we don't have a guarenteed mainloop, we must flush */
        dbus_connection_flush(bus);
        return true;

err:
        dbus_error_free(&error);
        return false;
    }
}

//! The main function
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printHelp();
        return EXIT_FAILURE;
    }
    else if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")
    {
        printHelp();
        return EXIT_SUCCESS;
    }
    else
    {
        if (!lock(argv[1]))
        {
            bool success = activateExistingInstance(argv[1]);
            if (!success)
            {
                return EXIT_FAILURE;
            }
        }
        else
        {
            if (execve(argv[1], argv + 1, environ) == -1)
            {
                report(report_error, "Failed to exec binary '%s' : %s\n", argv[1], strerror(errno));
                unlock();
                
                return EXIT_FAILURE;
            }
        }
    }
    
    return EXIT_SUCCESS;
}

