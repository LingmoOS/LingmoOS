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

#include "singleinstance.h"
#include <dlfcn.h>

bool SingleInstance::validateAndRegisterPlugin(void * handle)
{
    // Check if lock() is there
    dlerror();
    lock_func_t lock = (lock_func_t)dlsym(handle, "lock");

    // Check error
    char * error = NULL;
    if ((error = dlerror()) != NULL)
    {
        return false;
    }

    // Check if unlock() is there
    dlerror();
    unlock_func_t unlock = (unlock_func_t)dlsym(handle, "unlock");

    // Check error
    if ((error = dlerror()) != NULL)
    {
        return false;
    }

    // Check if activateExistingInstance() is there
    dlerror();
    activate_func_t activateExistingInstance =
            (activate_func_t)dlsym(handle, "activateExistingInstance");

    // Check error
    if ((error = dlerror()) != NULL)
    {
        return false;
    }

    // Register the plugin
    m_pluginEntry.reset(new SingleInstancePluginEntry);
    m_pluginEntry->handle = handle;
    m_pluginEntry->lockFunc = lock;
    m_pluginEntry->unlockFunc = unlock;
    m_pluginEntry->activateExistingInstanceFunc = activateExistingInstance;

    return true;
}

SingleInstancePluginEntry * SingleInstance::pluginEntry() const
{
    return m_pluginEntry.get();
}

void SingleInstance::closePlugin()
{
    if (m_pluginEntry)
    {
        dlclose(m_pluginEntry->handle);
        m_pluginEntry.reset();
    }
}
