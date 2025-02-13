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

#ifndef APPDATA_H
#define APPDATA_H

#include "launcherlib.h"
#include <stdint.h>
#include <sys/types.h>

#include <string>

using std::string;

#include <vector>

using std::vector;

typedef int (*entry_t)(int, char **);

//! Structure for application data read from the invoker
class DECL_EXPORT AppData
{
public:

    //! Constructor
    AppData();

    //! Destructor
    ~AppData();

    //! Set options
    void setOptions(uint32_t options);

    //! Return options
    int options() const;

    //! Return whether or not RTLD_GLOBAL should be used in dlopen
    bool dlopenGlobal() const;

    //! Return whether or not RTLD_DEEPBIND should be used in dlopen
    bool dlopenDeep() const;

    //! Return whether or not application should be launched as a single instance application
    bool singleInstance() const;

    //! Return whether or not disable default out of memory killing adjustments for application process 
    bool disableOutOfMemAdj() const;

    //! Set argument count
    void setArgc(int argc);

    //! Return argument count
    int argc() const;

    //! Set address of the argument vector
    void setArgv(const char ** argv);

    //! Prepend to argv
    void prependArgv(const char *arg);

    //! Return address of the argument vector
    const char ** argv() const;

    //! Set application name
    void setAppName(const string & appName);

    //! Return application name
    const string & appName() const;

    //! Set file name
    void setFileName(const string & fileName);

    //! Return file name
    const string & fileName() const;

    //! Set priority
    void setPriority(int priority);

    //! Return priority
    int priority() const;

    //! Set booster respawn delay
    void setDelay(int delay);

    //!Return respawn delay
    int delay() const;

    //! Set entry point for the application
    void setEntry(entry_t entry);

    //! Get the entry point
    entry_t entry() const;

    //! Get I/O descriptors
    const vector<int> & ioDescriptors() const;

    //! Set I/O descriptors
    void setIODescriptors(const vector<int> & ioDescriptors);

    //! Set user ID and group ID of calling process
    void setIDs(uid_t userId, gid_t groupId);

    //! Get user ID of calling process
    uid_t userId() const;

    //! Get group ID of calling process
    gid_t groupId() const;

    //! Returns true if the child process should be privileged
    bool isPrivileged() const;

    //! Get privilege string for this app
    string privileges() const;

private:

    AppData(const AppData & r);
    AppData & operator= (const AppData & r);
    string getPrivileges(const char *path);

    uint32_t    m_options;
    int         m_argc;
    char      **m_argv;
    string      m_appName;
    string      m_fileName;
    int         m_prio;
    int         m_delay;
    entry_t     m_entry;
    vector<int> m_ioDescriptors;
    gid_t       m_gid;
    uid_t       m_uid;
    string      m_privileges;
};

#endif // APPDATA_H
