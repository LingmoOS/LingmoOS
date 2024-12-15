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
#include <sys/types.h>

BEGIN_NAMESPACE
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
    m_argc = newArgc;
}

int AppData::argc() const
{
    return m_argc;
}

void AppData::setArgv(const char ** newArgv)
{
    m_argv = newArgv;
}

const char ** AppData::argv() const
{
    return m_argv;
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

AppData::~AppData()
{
}
END_NAMESPACE