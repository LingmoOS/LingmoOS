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

#include "appdata.h"
#include "protocol.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <fstream>
#include <dirent.h>
#include <string.h>

AppData::AppData() :
    m_options(0),
    m_argc(0),
    m_argv(NULL),
    m_appName(""),
    m_fileName(""),
    m_prio(0),
    m_delay(0),
    m_entry(NULL),
    m_ioDescriptors(),
    m_gid(0),
    m_uid(0)
{}

void AppData::setOptions(uint32_t newOptions)
{
    m_options = newOptions;
}

int AppData::options() const
{
    return m_options;
}

bool AppData::dlopenGlobal() const
{
    return (m_options & INVOKER_MSG_MAGIC_OPTION_DLOPEN_GLOBAL) != 0;
}

bool AppData::dlopenDeep() const
{
    return (m_options & INVOKER_MSG_MAGIC_OPTION_DLOPEN_DEEP) != 0;
}

bool AppData::singleInstance() const
{
    return (m_options & INVOKER_MSG_MAGIC_OPTION_SINGLE_INSTANCE) != 0;
}

bool AppData::disableOutOfMemAdj() const
{
    return (m_options & INVOKER_MSG_MAGIC_OPTION_OOM_ADJ_DISABLE) != 0;
}

void AppData::setArgc(int newArgc)
{
    (void)newArgc; // unused
    // kept for the sake of binary compatibility
    // setArgv() sets also m_argc
}

int AppData::argc() const
{
    return m_argc;
}

void AppData::setArgv(const char ** newArgv)
{
    for (int i = 0; i < m_argc; ++i)
        free(m_argv[i]);
    free(m_argv);
    m_argc = 0;
    m_argv = nullptr;

    if (newArgv) {
        while (newArgv[m_argc])
            ++m_argc;
        m_argv = (char **)calloc(m_argc + 1, sizeof *m_argv);
        for (int i = 0; i < m_argc; ++i)
            m_argv[i] = strdup(newArgv[i]);
        m_argv[m_argc] = nullptr;
    }
}

void AppData::prependArgv(const char * arg)
{
    char **oldArgv = m_argv;
    m_argv = (char **)calloc(++m_argc + 1, sizeof *m_argv);
    m_argv[0] = strdup(arg);
    for (int i = 1; i < m_argc + 1; ++i)
        m_argv[i] = oldArgv[i-1];
    free(oldArgv);
}

const char ** AppData::argv() const
{
    return (const char **)m_argv;
}

void AppData::setAppName(const string & newAppName)
{
    m_appName = newAppName;
}

const string & AppData::appName() const
{
    return m_appName;
}

void AppData::setFileName(const string & newFileName)
{
    m_fileName = newFileName;
}

const string & AppData::fileName() const
{
    return m_fileName;
}

void AppData::setPriority(int newPriority)
{
    m_prio = newPriority;
}

int AppData::priority() const
{
    return m_prio;
}

void AppData::setDelay(int newDelay)
{
    m_delay = newDelay;
}

int AppData::delay() const
{
    return m_delay;
}

void AppData::setEntry(entry_t newEntry)
{
    m_entry = newEntry;
}

entry_t AppData::entry() const
{
    return m_entry;
}

const vector<int> & AppData::ioDescriptors() const
{
    return m_ioDescriptors;
}

void AppData::setIODescriptors(const vector<int> & newIODescriptors)
{
    m_ioDescriptors = newIODescriptors;
}

void AppData::setIDs(uid_t userId, gid_t groupId)
{
    m_uid = userId;
    m_gid = groupId;
}

uid_t AppData::userId() const
{
    return m_uid;
}

gid_t AppData::groupId() const
{
    return m_gid;
}

string AppData::getPrivileges(const char *path)
{
    /*
       Returns string of the declared privileges for this app.
       The privileges file has the following format:
           /full/path/to/app,<permissions_list>
       where the permissions_list is a string of characters
       defining different categories of permissions
           eg: p = people/contacts data
       example:
           /usr/bin/vcardconverter,p
       Currently, permission means both read+write permission.
       Comment lines start with # and are ignored.
    */

    std::ifstream infile(path);
    std::string permissions;
    if (infile) {
        std::string line;
        while (std::getline(infile, line)) {
            if (line.find('#') == 0) {
                // Comment line
                continue;
            }

            size_t pos = line.find(',');
            if (pos != std::string::npos) {
                std::string filename = line.substr(0, pos);
                if (filename == m_fileName) {
                    if (pos == line.length()-1) {
                        // some privilege files don't include any privilege flag
                        // but still need to make the process privileged
                        permissions += " ";
                    }
                    else {
                        permissions += line.substr(pos+1);
                    }
                }
            }
        }
    }

    return permissions;
}

bool AppData::isPrivileged() const
{
    return m_privileges.length() > 0;
}

string AppData::privileges() const
{
    return m_privileges;
}


AppData::~AppData()
{
    setArgv(nullptr);
}
